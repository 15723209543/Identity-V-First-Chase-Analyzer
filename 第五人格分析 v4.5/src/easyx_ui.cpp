#include "easyx_ui.h"
#include "auxiliary_trait_data.h"
#include "data_repository.h"
#include "hunter_data.h"
#include "map_data.h"
#include "survivor_data.h"

#include <graphics.h>
#include <windows.h>

#include <algorithm>
#include <iomanip>
#include <map>
#include <queue>
#include <sstream>

namespace
{
    // 窗口被拉宽，左侧留给纯地图，右侧放所有说明、图例、列表和结果。
    const int k_window_width = 1960;
    const int k_window_height = 1020;
    const int k_map_x = 18;
    const int k_map_y = 18;
    const int k_map_w = 1088;
    const int k_map_h = 964;
    const int k_panel_x = 1128;
    const int k_panel_y = 18;
    const int k_panel_w = 808;
    const int k_panel_h = 964;
    const int k_back_value = -1;

    struct map_play_crop
    {
        int map_id;
        int left;
        int top;
        int right;
        int bottom;
    };

    // 这些裁剪框和 tools/generate_map_geometry.py 保持一致。
    // 它们对应用户提供图片里“实际可对局地图”的外接范围，
    // 可以把标题、图例、图片空白边和外墙外侧的大块白底排除掉。
    const map_play_crop k_map_play_crops[] = {
        { 1, 55, 270, 1288, 1372 },
        { 2, 50, 245, 1282, 1435 },
        { 3, 100, 220, 1262, 1430 },
        { 4, 130, 270, 1210, 1630 },
        { 5, 60, 310, 1265, 1410 },
        { 6, 80, 290, 1260, 1425 },
        { 7, 85, 285, 1288, 1438 },
        { 8, 60, 210, 1265, 928 },
        { 9, 60, 325, 1265, 1405 }
    };

    struct visual_walk_cache
    {
        bool valid = false;
        int width = 0;
        int height = 0;
        int play_left = 0;
        int play_top = 0;
        int play_right = 0;
        int play_bottom = 0;
        std::vector<unsigned char> white_pixels;
        std::vector<unsigned char> outside_white_pixels;
    };

    int scale_x(int value)
    {
        // 地图数据统一使用 0-1000 相对坐标。
        // 原图实际画在地图框内缩进 8px，所以角色标记也必须映射到同一块图片区域。
        return k_map_x + 8 + value * (k_map_w - 16) / 1000;
    }

    int scale_y(int value)
    {
        return k_map_y + 8 + value * (k_map_h - 16) / 1000;
    }

    int unscale_x(int value)
    {
        // 鼠标点击坐标反向换算为 0-1000 地图坐标，供寻路和区域判断使用。
        return (value - k_map_x - 8) * 1000 / (k_map_w - 16);
    }

    int unscale_y(int value)
    {
        return (value - k_map_y - 8) * 1000 / (k_map_h - 16);
    }

    bool inside_map_canvas(int x, int y)
    {
        // 图片实际绘制时在地图框内缩进了 8 像素。
        // 点击判定必须和图片绘制区域完全一致，否则边框附近会被错误换算成图内点。
        return x >= k_map_x + 8
            && x <= k_map_x + k_map_w - 8
            && y >= k_map_y + 8
            && y <= k_map_y + k_map_h - 8;
    }

    bool inside_area(const click_area& area, int x, int y)
    {
        return x >= area.left && x <= area.right && y >= area.top && y <= area.bottom;
    }

    bool is_near_white(COLORREF color)
    {
        // 用户要求“只有白色区域可以点击”。这里使用较高阈值，避免浅灰墙体、
        // 彩色窗板、地下室图标、地图文字、图例等被当成可走区域。
        return GetRValue(color) >= 238
            && GetGValue(color) >= 238
            && GetBValue(color) >= 238;
    }

    std::string percent_text(double value)
    {
        std::ostringstream out;
        out.setf(std::ios::fixed);
        out.precision(2);
        out << value * 100.0 << "%";
        return out.str();
    }

    std::vector<std::string> wrap_text_lines(int max_width, const std::string& value)
    {
        // 把 GBK 中文按“一个汉字两个字节”的方式拆成显示单元，再按 EasyX 实际文字宽度换行。
        // 结果页滚动时复用这份行数组，避免每次滚动都出现半个汉字或横向截断。
        settextstyle(16, 0, "微软雅黑");
        std::vector<std::string> lines;
        std::string line;
        for (size_t i = 0; i < value.size();)
        {
            std::string unit;
            const unsigned char current = static_cast<unsigned char>(value[i]);
            if (value[i] == '\r')
            {
                ++i;
                continue;
            }
            if (value[i] == '\n')
            {
                lines.push_back(line);
                line.clear();
                ++i;
                continue;
            }

            if (current >= 0x80 && i + 1 < value.size())
            {
                unit = value.substr(i, 2);
                i += 2;
            }
            else
            {
                unit = value.substr(i, 1);
                ++i;
            }

            const std::string candidate = line + unit;
            if (!line.empty() && textwidth(candidate.c_str()) > max_width)
            {
                lines.push_back(line);
                line = unit;
            }
            else
            {
                line = candidate;
            }
        }

        if (!line.empty())
        {
            lines.push_back(line);
        }
        return lines;
    }

    std::string map_image_path(const map_geometry_static_data& geometry)
    {
        // EasyX 的 loadimage 可以直接读取 jpg。
        // 路径只使用 ASCII 目录和文件名，避免中文工程目录在图片加载时产生编码歧义。
        return "data\\map\\" + geometry.source_file;
    }

    const map_play_crop* find_map_play_crop(int map_id)
    {
        for (const auto& crop : k_map_play_crops)
        {
            if (crop.map_id == map_id)
            {
                return &crop;
            }
        }
        return nullptr;
    }

    visual_walk_cache build_visual_walk_cache(const map_geometry_static_data& geometry)
    {
        // 点击判定使用原图像素，而不是屏幕缩放后的颜色。
        // 先找出所有近白色像素，再从图片四周向内泛洪；与边缘连通的白色都是图外空白，
        // 只有“位于有效地图裁剪范围内、白色且不连通到图片边缘”的部分才视为可点击空地。
        // 裁剪范围用来额外挡掉标题、图例、图片白边以及外墙外侧的大块白底。
        visual_walk_cache cache;
        IMAGE image;
        loadimage(&image, map_image_path(geometry).c_str());
        cache.width = image.getwidth();
        cache.height = image.getheight();
        if (cache.width <= 0 || cache.height <= 0)
        {
            return cache;
        }

        const map_play_crop* crop = find_map_play_crop(geometry.map_id);
        if (crop)
        {
            cache.play_left = (std::max)(0, (std::min)(cache.width - 1, crop->left));
            cache.play_top = (std::max)(0, (std::min)(cache.height - 1, crop->top));
            cache.play_right = (std::max)(0, (std::min)(cache.width - 1, crop->right));
            cache.play_bottom = (std::max)(0, (std::min)(cache.height - 1, crop->bottom));
        }
        else
        {
            cache.play_left = 0;
            cache.play_top = 0;
            cache.play_right = cache.width - 1;
            cache.play_bottom = cache.height - 1;
        }

        const int total = cache.width * cache.height;
        cache.white_pixels.assign(total, 0);
        cache.outside_white_pixels.assign(total, 0);

        DWORD* pixels = GetImageBuffer(&image);
        for (int i = 0; i < total; ++i)
        {
            if (is_near_white(static_cast<COLORREF>(pixels[i])))
            {
                cache.white_pixels[i] = 1;
            }
        }

        std::queue<int> queue;
        const auto push_if_white_border = [&](int x, int y)
        {
            const int index = y * cache.width + x;
            if (cache.white_pixels[index] && !cache.outside_white_pixels[index])
            {
                cache.outside_white_pixels[index] = 1;
                queue.push(index);
            }
        };

        for (int x = 0; x < cache.width; ++x)
        {
            push_if_white_border(x, 0);
            push_if_white_border(x, cache.height - 1);
        }
        for (int y = 0; y < cache.height; ++y)
        {
            push_if_white_border(0, y);
            push_if_white_border(cache.width - 1, y);
        }

        const int dx_values[4] = { 1, -1, 0, 0 };
        const int dy_values[4] = { 0, 0, 1, -1 };
        while (!queue.empty())
        {
            const int current = queue.front();
            queue.pop();
            const int current_x = current % cache.width;
            const int current_y = current / cache.width;
            for (int i = 0; i < 4; ++i)
            {
                const int next_x = current_x + dx_values[i];
                const int next_y = current_y + dy_values[i];
                if (next_x < 0 || next_x >= cache.width || next_y < 0 || next_y >= cache.height)
                {
                    continue;
                }

                const int next = next_y * cache.width + next_x;
                if (cache.white_pixels[next] && !cache.outside_white_pixels[next])
                {
                    cache.outside_white_pixels[next] = 1;
                    queue.push(next);
                }
            }
        }

        cache.valid = true;
        return cache;
    }

    const visual_walk_cache* visual_cache_for_map(const game_map& map)
    {
        const map_geometry_static_data* geometry = map.geometry();
        if (!geometry)
        {
            return nullptr;
        }

        static std::map<int, visual_walk_cache> caches;
        auto cache_iter = caches.find(map.id());
        if (cache_iter == caches.end())
        {
            cache_iter = caches.emplace(map.id(), build_visual_walk_cache(*geometry)).first;
        }

        return cache_iter->second.valid ? &cache_iter->second : nullptr;
    }

    bool is_yongmian_visual_forced_open(int x, int y)
    {
        // 这里的坐标已经是有效地图裁剪区内的 0-1000 相对坐标，
        // 与 game_map.cpp 里的永眠镇通行修正保持同一套口径。
        const bool left_lane = x >= 85 && x <= 155 && y >= 115 && y <= 685;
        const bool middle_road = x >= 445 && x <= 590 && y >= 250 && y <= 455;
        return left_lane || middle_road;
    }

    int scale_x_for_map(const game_map& map, int value)
    {
        const visual_walk_cache* cache = visual_cache_for_map(map);
        if (!cache)
        {
            return scale_x(value);
        }

        const int image_left = k_map_x + 8;
        const int image_width = k_map_w - 16;
        const int crop_left = image_left + cache->play_left * image_width / cache->width;
        const int crop_width = (cache->play_right - cache->play_left) * image_width / cache->width;
        return crop_left + value * crop_width / 1000;
    }

    int scale_y_for_map(const game_map& map, int value)
    {
        const visual_walk_cache* cache = visual_cache_for_map(map);
        if (!cache)
        {
            return scale_y(value);
        }

        const int image_top = k_map_y + 8;
        const int image_height = k_map_h - 16;
        const int crop_top = image_top + cache->play_top * image_height / cache->height;
        const int crop_height = (cache->play_bottom - cache->play_top) * image_height / cache->height;
        return crop_top + value * crop_height / 1000;
    }

    bool screen_to_map_position(const game_map& map, int screen_x, int screen_y, int& x, int& y)
    {
        const visual_walk_cache* cache = visual_cache_for_map(map);
        if (!cache)
        {
            x = (std::max)(0, (std::min)(1000, unscale_x(screen_x)));
            y = (std::max)(0, (std::min)(1000, unscale_y(screen_y)));
            return true;
        }

        const int image_left = k_map_x + 8;
        const int image_top = k_map_y + 8;
        const int image_width = k_map_w - 16;
        const int image_height = k_map_h - 16;
        const int crop_left = image_left + cache->play_left * image_width / cache->width;
        const int crop_top = image_top + cache->play_top * image_height / cache->height;
        const int crop_right = image_left + cache->play_right * image_width / cache->width;
        const int crop_bottom = image_top + cache->play_bottom * image_height / cache->height;

        if (screen_x < crop_left || screen_x > crop_right || screen_y < crop_top || screen_y > crop_bottom)
        {
            return false;
        }

        x = (std::max)(0, (std::min)(1000, (screen_x - crop_left) * 1000 / (std::max)(1, crop_right - crop_left)));
        y = (std::max)(0, (std::min)(1000, (screen_y - crop_top) * 1000 / (std::max)(1, crop_bottom - crop_top)));
        return true;
    }

    bool is_visual_walkable_point(const game_map& map, int x, int y)
    {
        const visual_walk_cache* cache = visual_cache_for_map(map);
        if (!cache)
        {
            return false;
        }

        const int crop_width = (std::max)(1, cache->play_right - cache->play_left);
        const int crop_height = (std::max)(1, cache->play_bottom - cache->play_top);
        const int pixel_x = (std::max)(0, (std::min)(cache->width - 1, cache->play_left + x * crop_width / 1000));
        const int pixel_y = (std::max)(0, (std::min)(cache->height - 1, cache->play_top + y * crop_height / 1000));

        if (map.id() == 4 && is_yongmian_visual_forced_open(x, y))
        {
            return true;
        }

        const int index = pixel_y * cache->width + pixel_x;
        return cache->white_pixels[index] && !cache->outside_white_pixels[index];
    }
}

easyx_ui::easyx_ui()
{
}

easyx_ui::~easyx_ui()
{
    closegraph();
}

void easyx_ui::initialize()
{
    initgraph(k_window_width, k_window_height);
    setbkcolor(RGB(246, 247, 249));
    cleardevice();
    setbkmode(TRANSPARENT);
}

number_input_result easyx_ui::ask_map()
{
    draw_base("第一步：选择本局地图");
    draw_map(nullptr, {});
    draw_map_list();
    return wait_for_number_click("请用鼠标点击本局地图。", 1, max_map_id());
}

position_input_result easyx_ui::ask_position(const game_map& map,
    const std::string& title,
    const std::vector<map_marker>& markers,
    const std::string& back_text)
{
    draw_base(title);
    draw_map(&map, markers);
    text(k_panel_x + 18, k_panel_y + 18, title);
    text_small(k_panel_x + 18, k_panel_y + 62, "请点击左侧地图外墙内部的白色空地区域。");
    text_small(k_panel_x + 18, k_panel_y + 92, "墙体、建筑、窗板、地下室、文字、图例和外墙外侧都不可选择。");
    text_small(k_panel_x + 18, k_panel_y + 122, "选好位置后，再到右侧角色列表选择角色。");
    draw_back_button(back_text);
    return wait_for_position_click(map, "请点击左侧地图空地选择位置。");
}

number_input_result easyx_ui::ask_survivor(const game_map& map, int index, const std::vector<map_marker>& markers)
{
    std::ostringstream title;
    title << "第" << index << "名求生者：选择角色";
    draw_base(title.str());
    draw_map(&map, markers);
    click_areas_.clear();
    text(k_panel_x + 18, k_panel_y + 18, title.str());
    draw_survivor_list();
    draw_back_button("撤销位置");
    return wait_for_number_click("请用鼠标点击右侧求生者编号。", 1, max_survivor_id());
}

number_input_result easyx_ui::ask_hunter(const game_map& map, const std::vector<map_marker>& markers)
{
    draw_base("监管者：选择角色");
    draw_map(&map, markers);
    click_areas_.clear();
    text(k_panel_x + 18, k_panel_y + 18, "监管者：选择角色");
    draw_hunter_list();
    draw_back_button("撤销位置");
    return wait_for_number_click("请用鼠标点击右侧监管者编号。", 1, max_hunter_id());
}

number_input_result easyx_ui::ask_auxiliary_trait(const game_map& map, const std::vector<map_marker>& markers)
{
    draw_base("监管者：选择辅助技能");
    draw_map(&map, markers);
    click_areas_.clear();
    text(k_panel_x + 18, k_panel_y + 18, "监管者：选择辅助技能");
    text_small(k_panel_x + 18, k_panel_y + 56, "辅助技能会影响到达后追击节奏和预估击倒时间。");
    draw_auxiliary_trait_list();
    draw_back_button("重选监管者");
    return wait_for_number_click("请用鼠标点击右侧辅助技能编号。", 1, max_auxiliary_trait_id());
}

void easyx_ui::draw_result(const analysis_result& result, const std::string& report_text, const std::string& result_file_name)
{
    result_cache_ = result;
    result_report_text_ = report_text;
    result_file_name_ = result_file_name;
    result_scroll_line_ = 0;
    result_max_scroll_line_ = 0;
    draw_result_page();
}

void easyx_ui::draw_result_page()
{
    std::vector<map_marker> markers;
    const unsigned long colors[] = {
        RGB(26, 115, 232),
        RGB(19, 150, 91),
        RGB(245, 124, 0),
        RGB(142, 68, 173)
    };
    const analysis_result& result = result_cache_;
    const std::string& report_text = result_report_text_;
    const std::string& result_file_name = result_file_name_;

    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        std::ostringstream label;
        label << "S" << (i + 1);
        markers.push_back({
            result.survivors[i].region_id(),
            label.str(),
            result.survivors[i].name(),
            colors[i % 4],
            result.survivors[i].position_x(),
            result.survivors[i].position_y(),
            true
            });
    }
    markers.push_back({
        result.hunter_region_id,
        "H",
        result.selected_hunter.name(),
        RGB(210, 35, 35),
        result.hunter_x,
        result.hunter_y,
        true
        });

    draw_base("分析结果");
    draw_map(&result.selected_map, markers);

    setfillcolor(RGB(255, 255, 255));
    solidrectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);
    setlinecolor(RGB(210, 215, 222));
    rectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);

    text(k_panel_x + 22, k_panel_y + 22, "监管者首追概率");
    if (result.best_index >= 0 && result.best_index < static_cast<int>(result.survivors.size()))
    {
        std::string best = "最可能首追：" + result.survivors[result.best_index].name()
            + "（" + percent_text(result.scores[result.best_index].probability) + "）";
        text_small(k_panel_x + 22, k_panel_y + 60, best);
    }

    int y = k_panel_y + 92;
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const int bar_x = k_panel_x + 112;
        const int bar_y = y + 5;
        const int bar_w = 260;
        const int fill_w = static_cast<int>(bar_w * result.scores[i].probability);

        std::ostringstream name;
        name << "S" << (i + 1) << " " << result.survivors[i].name();
        text_small(k_panel_x + 22, y, name.str());
        setfillcolor(RGB(232, 236, 241));
        solidrectangle(bar_x, bar_y, bar_x + bar_w, bar_y + 16);
        setfillcolor(static_cast<COLORREF>(colors[i % 4]));
        solidrectangle(bar_x, bar_y, bar_x + fill_w, bar_y + 16);
        text_small(bar_x + bar_w + 8, y, percent_text(result.scores[i].probability));
        y += 42;
    }

    y += 12;
    setlinecolor(RGB(226, 230, 236));
    line(k_panel_x + 22, y, k_panel_x + k_panel_w - 22, y);
    y += 18;
    text(k_panel_x + 22, y, "完整分析");
    y += 34;

    const int text_x = k_panel_x + 22;
    const int text_top = y;
    const int text_width = k_panel_w - 62;
    const int text_bottom = k_panel_y + k_panel_h - 116;
    const int line_height = 21;
    const int visible_lines = (std::max)(1, (text_bottom - text_top) / line_height);
    const std::vector<std::string> lines = wrap_text_lines(text_width, report_text);
    result_max_scroll_line_ = (std::max)(0, static_cast<int>(lines.size()) - visible_lines);
    result_scroll_line_ = (std::max)(0, (std::min)(result_scroll_line_, result_max_scroll_line_));

    settextcolor(RGB(34, 40, 49));
    settextstyle(16, 0, "微软雅黑");
    for (int i = 0; i < visible_lines && i + result_scroll_line_ < static_cast<int>(lines.size()); ++i)
    {
        outtextxy(text_x, text_top + i * line_height, lines[i + result_scroll_line_].c_str());
    }

    if (result_max_scroll_line_ > 0)
    {
        const int track_x = k_panel_x + k_panel_w - 24;
        const int track_top = text_top;
        const int track_bottom = text_bottom;
        const int track_h = track_bottom - track_top;
        const int thumb_h = (std::max)(38, track_h * visible_lines / static_cast<int>(lines.size()));
        const int thumb_y = track_top + (track_h - thumb_h) * result_scroll_line_ / (std::max)(1, result_max_scroll_line_);
        setlinecolor(RGB(225, 230, 236));
        line(track_x, track_top, track_x, track_bottom);
        setfillcolor(RGB(165, 174, 186));
        solidrectangle(track_x - 4, thumb_y, track_x + 4, thumb_y + thumb_h);
        text_small(k_panel_x + 22, k_panel_y + k_panel_h - 96, "鼠标滚轮可查看完整分析。");
    }

    text_small(k_panel_x + 22, k_panel_y + k_panel_h - 70, "本次结果文件：" + result_file_name);
    text_small(k_panel_x + 22, k_panel_y + k_panel_h - 44, "点击任意位置结束。");
}

void easyx_ui::wait_for_finish()
{
    while (true)
    {
        ExMessage message = getmessage(EX_MOUSE);
        if (message.message == WM_MOUSEWHEEL && result_max_scroll_line_ > 0)
        {
            result_scroll_line_ += message.wheel > 0 ? -3 : 3;
            result_scroll_line_ = (std::max)(0, (std::min)(result_scroll_line_, result_max_scroll_line_));
            draw_result_page();
            continue;
        }
        if (message.message == WM_LBUTTONDOWN)
        {
            return;
        }
    }
}

number_input_result easyx_ui::wait_for_number_click(const std::string& prompt, int min_value, int max_value)
{
    std::string error;
    draw_prompt(prompt, error);

    while (true)
    {
        ExMessage message = getmessage(EX_MOUSE);
        if (message.message != WM_LBUTTONDOWN)
        {
            continue;
        }

        for (auto area = click_areas_.rbegin(); area != click_areas_.rend(); ++area)
        {
            if (inside_area(*area, message.x, message.y) && area->value == k_back_value)
            {
                return { input_action::back, 0 };
            }
            if (inside_area(*area, message.x, message.y)
                && area->value >= min_value
                && area->value <= max_value)
            {
                return { input_action::select, area->value };
            }
        }

        error = "没有点中可选项目，请重新点击。";
        draw_prompt(prompt, error);
    }
}

position_input_result easyx_ui::wait_for_position_click(const game_map& map, const std::string& prompt)
{
    std::string error;
    draw_prompt(prompt, error);

    while (true)
    {
        ExMessage message = getmessage(EX_MOUSE);
        if (message.message != WM_LBUTTONDOWN)
        {
            continue;
        }

        for (auto area = click_areas_.rbegin(); area != click_areas_.rend(); ++area)
        {
            // 位置输入阶段只有撤销按钮走 click_area；
            // 地图本身不登记为矩形按钮，而是直接读取鼠标坐标，精度更高。
            if (inside_area(*area, message.x, message.y) && area->value == k_back_value)
            {
                return { input_action::back, 0, 0, 0 };
            }
        }

        if (!inside_map_canvas(message.x, message.y))
        {
            error = "请点击左侧地图区域。";
            draw_prompt(prompt, error);
            continue;
        }

        int x = 0;
        int y = 0;
        if (!screen_to_map_position(map, message.x, message.y, x, y))
        {
            error = "请点击左侧地图外墙内部的白色空地。";
            draw_prompt(prompt, error);
            continue;
        }

        // 先用原图像素做显示层判定：必须点到外墙内的白色空地。
        // 再用几何网格做算法层判定：不能落到内部墙体或不可走障碍上。
        if (!is_visual_walkable_point(map, x, y) || map.is_blocked_point(x, y))
        {
            error = "这里只能选择外墙内部的白色空地，请重新点击。";
            draw_prompt(prompt, error);
            continue;
        }

        return { input_action::select, map.region_id_at(x, y), x, y };
    }
}

void easyx_ui::add_click_area(int left, int top, int right, int bottom, int value)
{
    click_areas_.push_back({ left, top, right, bottom, value });
}

void easyx_ui::draw_base(const std::string& title)
{
    (void)title;
    click_areas_.clear();
    cleardevice();
    setfillcolor(RGB(242, 244, 247));
    solidrectangle(0, 0, k_window_width, k_window_height);

    setfillcolor(RGB(255, 255, 255));
    solidrectangle(k_map_x, k_map_y, k_map_x + k_map_w, k_map_y + k_map_h);
    solidrectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);

    setlinecolor(RGB(210, 215, 222));
    rectangle(k_map_x, k_map_y, k_map_x + k_map_w, k_map_y + k_map_h);
    rectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);
}

void easyx_ui::draw_map(const game_map* map, const std::vector<map_marker>& markers)
{
    setfillcolor(RGB(252, 253, 255));
    solidrectangle(k_map_x + 8, k_map_y + 8, k_map_x + k_map_w - 8, k_map_y + k_map_h - 8);

    if (!map)
    {
        setlinecolor(RGB(180, 187, 197));
        rectangle(k_map_x + 40, k_map_y + 60, k_map_x + k_map_w - 40, k_map_y + k_map_h - 60);
        return;
    }

    const map_geometry_static_data* geometry = map->geometry();
    if (geometry)
    {
        // 地图主体直接使用用户提供的原图。
        // 这样墙体、窗、板、地下室、建筑外轮廓和图中中文标注都保持原封不动，
        // 不会再出现由网格边界推导造成的线条相交、断裂或形状失真。
        IMAGE map_image;
        loadimage(&map_image, map_image_path(*geometry).c_str(), k_map_w - 16, k_map_h - 16);
        putimage(k_map_x + 8, k_map_y + 8, &map_image);

    }
    else
    {
        for (const auto& region : map->regions())
        {
            const int left = scale_x(region.x);
            const int top = scale_y(region.y);
            const int right = scale_x(region.x + region.width);
            const int bottom = scale_y(region.y + region.height);
            setlinecolor(RGB(210, 215, 222));
            rectangle(left, top, right, bottom);
        }
    }

    std::map<int, int> region_marker_count;
    for (const auto& marker : markers)
    {
        const map_region_data* region = map->find_region(marker.region_id);
        if (!region)
        {
            continue;
        }

        const int index = region_marker_count[marker.region_id]++;
        // 同一区域可能有多个角色，index 用来做轻微偏移，避免 S1/S2/H 完全重叠。
        const int cx = marker.has_position
            ? scale_x_for_map(*map, marker.x) + index * 12
            : scale_x_for_map(*map, region->x + region->width / 2) + index * 24 - 12;
        const int cy = marker.has_position
            ? scale_y_for_map(*map, marker.y)
            : scale_y_for_map(*map, region->y + region->height / 2);

        if (!marker.display_name.empty())
        {
            settextstyle(16, 0, "微软雅黑");
            const int name_width = textwidth(marker.display_name.c_str());
            const int name_left = cx - name_width / 2 - 6;
            const int name_top = cy - 48;
            setfillcolor(RGB(255, 255, 255));
            solidrectangle(name_left, name_top, name_left + name_width + 12, name_top + 22);
            setlinecolor(static_cast<COLORREF>(marker.color));
            rectangle(name_left, name_top, name_left + name_width + 12, name_top + 22);
            settextcolor(static_cast<COLORREF>(marker.color));
            outtextxy(name_left + 6, name_top + 3, marker.display_name.c_str());
        }

        setfillcolor(static_cast<COLORREF>(marker.color));
        solidcircle(cx, cy, 15);
        setlinecolor(RGB(255, 255, 255));
        circle(cx, cy, 15);
        settextcolor(RGB(255, 255, 255));
        settextstyle(16, 0, "微软雅黑");
        outtextxy(cx - textwidth(marker.label.c_str()) / 2, cy - 8, marker.label.c_str());
        settextcolor(RGB(34, 40, 49));
    }
}

void easyx_ui::draw_map_list()
{
    text(k_panel_x + 18, k_panel_y + 18, "地图编号");
    text_small(k_panel_x + 18, k_panel_y + 54, "请选择本局地图。左侧将在选择后显示几何地图。");
    int y = k_panel_y + 92;
    for (const auto& data : k_map_data)
    {
        std::ostringstream line;
        line << data.id << " - " << data.name;
        add_click_area(k_panel_x + 18, y - 6, k_panel_x + k_panel_w - 22, y + 28, data.id);
        setfillcolor(RGB(248, 250, 252));
        solidrectangle(k_panel_x + 18, y - 6, k_panel_x + k_panel_w - 22, y + 28);
        setlinecolor(RGB(226, 230, 236));
        rectangle(k_panel_x + 18, y - 6, k_panel_x + k_panel_w - 22, y + 28);
        text_small(k_panel_x + 30, y + 2, line.str());
        y += 42;
    }
}

void easyx_ui::draw_survivor_list()
{
    text(k_panel_x + 18, k_panel_y + 84, "求生者编号");
    const int col_width = 155;
    const int row_height = 24;
    const int rows_per_column = 18;
    for (size_t i = 0; i < k_survivor_data.size(); ++i)
    {
        const int col = static_cast<int>(i) / rows_per_column;
        const int row = static_cast<int>(i) % rows_per_column;
        const int x = k_panel_x + 18 + col * col_width;
        const int y = k_panel_y + 122 + row * row_height;

        std::ostringstream line;
        line << k_survivor_data[i].id << "." << k_survivor_data[i].name;
        add_click_area(x - 4, y - 3, x + col_width - 10, y + 20, k_survivor_data[i].id);
        text_small(x, y, line.str());
    }
}

void easyx_ui::draw_hunter_list()
{
    text(k_panel_x + 18, k_panel_y + 84, "监管者编号");
    const int col_width = 225;
    const int row_height = 24;
    const int rows_per_column = 18;
    for (size_t i = 0; i < k_hunter_data.size(); ++i)
    {
        const int col = static_cast<int>(i) / rows_per_column;
        const int row = static_cast<int>(i) % rows_per_column;
        const int x = k_panel_x + 18 + col * col_width;
        const int y = k_panel_y + 122 + row * row_height;

        std::ostringstream line;
        line << k_hunter_data[i].id << "." << k_hunter_data[i].name;
        add_click_area(x - 4, y - 3, x + col_width - 10, y + 20, k_hunter_data[i].id);
        text_small(x, y, line.str());
    }
}

void easyx_ui::draw_auxiliary_trait_list()
{
    text(k_panel_x + 18, k_panel_y + 96, "辅助技能编号");
    const int row_height = 74;
    int y = k_panel_y + 136;
    for (const auto& data : k_auxiliary_trait_data)
    {
        add_click_area(k_panel_x + 18, y - 5, k_panel_x + k_panel_w - 24, y + row_height - 12, data.id);
        setfillcolor(RGB(248, 250, 252));
        solidrectangle(k_panel_x + 18, y - 5, k_panel_x + k_panel_w - 24, y + row_height - 12);
        setlinecolor(RGB(226, 230, 236));
        rectangle(k_panel_x + 18, y - 5, k_panel_x + k_panel_w - 24, y + row_height - 12);

        std::ostringstream line;
        line.setf(std::ios::fixed);
        line.precision(0);
        line << data.id << "." << data.name
            << "（开局CD " << data.opening_cooldown_seconds << "秒）";
        text_small(k_panel_x + 30, y + 4, line.str());
        draw_wrapped_text(k_panel_x + 42, y + 28, k_panel_w - 76, y + row_height - 16, data.effect, 18);
        y += row_height;
    }
}

void easyx_ui::draw_region_list(const game_map& map)
{
    text(k_panel_x + 18, k_panel_y + 18, "区域编号");
    const int columns = 2;
    const int item_width = (k_panel_w - 54) / columns;
    const int row_height = 78;
    for (size_t i = 0; i < map.regions().size(); ++i)
    {
        const auto& region = map.regions()[i];
        const int col = static_cast<int>(i) % columns;
        const int row = static_cast<int>(i) / columns;
        const int x = k_panel_x + 22 + col * item_width;
        const int y = k_panel_y + 58 + row * row_height;

        std::ostringstream line;
        line << region.id << " - " << region.name
            << "（牵制" << region.kite_score
            << "，开阔" << region.openness << "）";
        add_click_area(x - 4, y - 4, x + item_width - 12, y + row_height - 8, region.id);
        setlinecolor(RGB(226, 230, 236));
        rectangle(x - 4, y - 4, x + item_width - 12, y + row_height - 8);
        text_small(x, y, line.str());
        draw_wrapped_text(x + 12, y + 24, item_width - 22, y + row_height - 6, region.note, 18);
    }
}

void easyx_ui::draw_map_information(const game_map& map, const std::string& mode_title)
{
    text(k_panel_x + 18, k_panel_y + 18, mode_title);
    text_small(k_panel_x + 18, k_panel_y + 58, "地图：" + map.name());

    const map_geometry_static_data* geometry = map.geometry();
    if (geometry)
    {
        std::ostringstream scale_line;
        scale_line.setf(std::ios::fixed);
        scale_line.precision(0);
        scale_line << "寻路尺度：" << geometry->real_width_m << "m × " << geometry->real_height_m << "m";
        text_small(k_panel_x + 18, k_panel_y + 84, scale_line.str());
    }

    draw_legend(k_panel_x + 18, k_panel_y + 116);
}

void easyx_ui::draw_legend(int x, int y)
{
    text_small(x, y, "图例");

    setlinecolor(RGB(168, 176, 186));
    setlinestyle(PS_SOLID, 2);
    rectangle(x, y + 30, x + 34, y + 48);
    setlinestyle(PS_SOLID, 1);
    text_small(x + 48, y + 28, "墙体/建筑：不可走");

    setlinecolor(RGB(245, 195, 50));
    setlinestyle(PS_SOLID, 4);
    line(x, y + 72, x + 34, y + 72);
    setlinestyle(PS_SOLID, 1);
    text_small(x + 48, y + 62, "窗：可牵制资源");

    setlinecolor(RGB(40, 170, 75));
    setlinestyle(PS_SOLID, 4);
    line(x, y + 102, x + 34, y + 102);
    setlinestyle(PS_SOLID, 1);
    text_small(x + 48, y + 92, "木板：可牵制资源");

    text_small(x + 48, y + 122, "大门/小门：以原图门图标为准");
}

void easyx_ui::draw_back_button(const std::string& text_value)
{
    const int left = k_panel_x + 18;
    const int top = k_panel_y + k_panel_h - 120;
    const int right = k_panel_x + 190;
    const int bottom = top + 38;

    add_click_area(left, top, right, bottom, k_back_value);
    setfillcolor(RGB(245, 247, 250));
    solidrectangle(left, top, right, bottom);
    setlinecolor(RGB(180, 187, 197));
    rectangle(left, top, right, bottom);
    text_small(left + 18, top + 8, text_value);
}

void easyx_ui::draw_prompt(const std::string& prompt, const std::string& error)
{
    const int top = k_panel_y + 780;
    const int bottom = k_panel_y + k_panel_h - 132;
    setfillcolor(RGB(255, 255, 255));
    solidrectangle(k_panel_x + 8, top, k_panel_x + k_panel_w - 8, bottom);

    text_small(k_panel_x + 18, top + 10, prompt);
    if (!error.empty())
    {
        settextcolor(RGB(200, 45, 45));
        text_small(k_panel_x + 18, top + 40, error);
        settextcolor(RGB(34, 40, 49));
    }
}

int easyx_ui::draw_wrapped_text(int x, int y, int max_width, int max_y, const std::string& value, int line_height)
{
    settextcolor(RGB(34, 40, 49));
    settextstyle(16, 0, "微软雅黑");

    std::string line;
    for (size_t i = 0; i < value.size();)
    {
        std::string unit;
        const unsigned char current = static_cast<unsigned char>(value[i]);
        if (value[i] == '\r')
        {
            ++i;
            continue;
        }
        if (value[i] == '\n')
        {
            if (y + line_height > max_y)
            {
                text_small(x, y, "……");
                return y + line_height;
            }
            outtextxy(x, y, line.c_str());
            line.clear();
            y += line_height;
            ++i;
            continue;
        }

        if (current >= 0x80 && i + 1 < value.size())
        {
            unit = value.substr(i, 2);
            i += 2;
        }
        else
        {
            unit = value.substr(i, 1);
            ++i;
        }

        const std::string candidate = line + unit;
        if (!line.empty() && textwidth(candidate.c_str()) > max_width)
        {
            if (y + line_height > max_y)
            {
                text_small(x, y, "……");
                return y + line_height;
            }
            outtextxy(x, y, line.c_str());
            line = unit;
            y += line_height;
        }
        else
        {
            line = candidate;
        }
    }

    if (!line.empty() && y + line_height <= max_y)
    {
        outtextxy(x, y, line.c_str());
        y += line_height;
    }
    else if (!line.empty())
    {
        text_small(x, y, "……");
        y += line_height;
    }
    return y;
}

void easyx_ui::text(int x, int y, const std::string& value)
{
    settextcolor(RGB(34, 40, 49));
    settextstyle(22, 0, "微软雅黑");
    outtextxy(x, y, value.c_str());
}

void easyx_ui::text_small(int x, int y, const std::string& value)
{
    settextcolor(RGB(34, 40, 49));
    settextstyle(16, 0, "微软雅黑");
    outtextxy(x, y, value.c_str());
}
