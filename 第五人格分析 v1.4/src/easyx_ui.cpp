#include "easyx_ui.h"
#include "data_repository.h"
#include "hunter_data.h"
#include "map_data.h"
#include "survivor_data.h"

#include <graphics.h>
#include <windows.h>

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

namespace
{
    const int k_window_width = 1720;
    const int k_window_height = 920;
    const int k_map_x = 20;
    const int k_map_y = 20;
    const int k_map_w = 930;
    const int k_map_h = 860;
    const int k_panel_x = 980;
    const int k_panel_y = 20;
    const int k_panel_w = 710;
    const int k_panel_h = 860;

    int scale_x(int value)
    {
        return k_map_x + value * k_map_w / 1000;
    }

    int scale_y(int value)
    {
        return k_map_y + 40 + value * (k_map_h - 60) / 1000;
    }

    bool inside_area(const click_area& area, int x, int y)
    {
        return x >= area.left && x <= area.right && y >= area.top && y <= area.bottom;
    }

    std::string percent_text(double value)
    {
        std::ostringstream out;
        out.setf(std::ios::fixed);
        out.precision(2);
        out << value * 100.0 << "%";
        return out.str();
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

int easyx_ui::ask_map()
{
    draw_base("第一步：选择本局地图");
    draw_map(nullptr, {});
    draw_map_list();
    return wait_for_click("请用鼠标点击本局地图。", 1, max_map_id());
}

int easyx_ui::ask_region(const game_map& map, const std::string& title, const std::vector<map_marker>& markers)
{
    draw_base(title);
    draw_map(&map, markers);
    draw_region_list(map);
    return wait_for_click("请点击左侧地图区域，或点击右侧区域编号。", 1, static_cast<int>(map.regions().size()));
}

int easyx_ui::ask_survivor(const game_map& map, int index, const std::vector<map_marker>& markers)
{
    std::ostringstream title;
    title << "第" << index << "名求生者：选择角色";
    draw_base(title.str());
    draw_map(&map, markers);
    draw_survivor_list();
    return wait_for_click("请用鼠标点击求生者编号。", 1, max_survivor_id());
}

int easyx_ui::ask_hunter(const game_map& map, const std::vector<map_marker>& markers)
{
    draw_base("监管者：选择角色");
    draw_map(&map, markers);
    draw_hunter_list();
    return wait_for_click("请用鼠标点击监管者编号。", 1, max_hunter_id());
}

void easyx_ui::draw_result(const analysis_result& result, const std::string& report_text, const std::string& result_file_name)
{
    std::vector<map_marker> markers;
    const unsigned long colors[] = {
        RGB(26, 115, 232),
        RGB(19, 150, 91),
        RGB(245, 124, 0),
        RGB(142, 68, 173)
    };
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        std::ostringstream label;
        label << "S" << (i + 1);
        markers.push_back({ result.survivors[i].region_id(), label.str(), result.survivors[i].name(), colors[i % 4] });
    }
    markers.push_back({ result.hunter_region_id, "H", result.selected_hunter.name(), RGB(210, 35, 35) });

    draw_base("分析结果");
    draw_map(&result.selected_map, markers);

    setfillcolor(RGB(255, 255, 255));
    solidrectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);
    setlinecolor(RGB(210, 215, 222));
    rectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);

    text(k_panel_x + 18, k_panel_y + 20, "监管者首追概率");
    if (result.best_index >= 0 && result.best_index < static_cast<int>(result.survivors.size()))
    {
        std::string best = "最可能首追：" + result.survivors[result.best_index].name()
            + "（" + percent_text(result.scores[result.best_index].probability) + "）";
        text_small(k_panel_x + 18, k_panel_y + 55, best);
    }

    int y = k_panel_y + 100;
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const int bar_x = k_panel_x + 125;
        const int bar_y = y + 4;
        const int bar_w = 300;
        const int fill_w = static_cast<int>(bar_w * result.scores[i].probability);

        std::ostringstream name;
        name << "S" << (i + 1) << " " << result.survivors[i].name();
        text_small(k_panel_x + 18, y, name.str());
        setfillcolor(RGB(232, 236, 241));
        solidrectangle(bar_x, bar_y, bar_x + bar_w, bar_y + 18);
        setfillcolor(static_cast<COLORREF>(colors[i % 4]));
        solidrectangle(bar_x, bar_y, bar_x + fill_w, bar_y + 18);
        text_small(bar_x + bar_w + 8, y, percent_text(result.scores[i].probability));
        y += 48;
    }

    y += 12;
    setlinecolor(RGB(226, 230, 236));
    line(k_panel_x + 18, y, k_panel_x + k_panel_w - 18, y);
    y += 18;
    text(k_panel_x + 18, y, "完整分析");
    y += 34;
    draw_wrapped_text(k_panel_x + 18, y, k_panel_w - 36, k_panel_y + k_panel_h - 52, report_text, 20);
    text_small(k_panel_x + 18, k_panel_y + k_panel_h - 54, "本次结果文件：" + result_file_name);
    text_small(k_panel_x + 18, k_panel_y + k_panel_h - 30, "点击任意位置结束。");
}

void easyx_ui::wait_for_finish()
{
    while (true)
    {
        ExMessage message = getmessage(EX_MOUSE);
        if (message.message == WM_LBUTTONDOWN)
        {
            return;
        }
    }
}

int easyx_ui::wait_for_click(const std::string& prompt, int min_value, int max_value)
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
            if (inside_area(*area, message.x, message.y)
                && area->value >= min_value
                && area->value <= max_value)
            {
                return area->value;
            }
        }

        error = "没有点中可选项目，请重新点击。";
        draw_prompt(prompt, error);
    }
}

void easyx_ui::add_click_area(int left, int top, int right, int bottom, int value)
{
    click_areas_.push_back({ left, top, right, bottom, value });
}

void easyx_ui::draw_base(const std::string& title)
{
    click_areas_.clear();
    cleardevice();
    setfillcolor(RGB(246, 247, 249));
    solidrectangle(0, 0, k_window_width, k_window_height);

    setfillcolor(RGB(255, 255, 255));
    solidrectangle(k_map_x, k_map_y, k_map_x + k_map_w, k_map_y + k_map_h);
    solidrectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);

    setlinecolor(RGB(210, 215, 222));
    rectangle(k_map_x, k_map_y, k_map_x + k_map_w, k_map_y + k_map_h);
    rectangle(k_panel_x, k_panel_y, k_panel_x + k_panel_w, k_panel_y + k_panel_h);

    text(k_map_x + 18, k_map_y + 12, title);
}

void easyx_ui::draw_map(const game_map* map, const std::vector<map_marker>& markers)
{
    setfillcolor(RGB(252, 252, 253));
    solidrectangle(k_map_x + 10, k_map_y + 45, k_map_x + k_map_w - 10, k_map_y + k_map_h - 10);

    if (!map)
    {
        setlinecolor(RGB(180, 187, 197));
        rectangle(k_map_x + 40, k_map_y + 100, k_map_x + k_map_w - 40, k_map_y + k_map_h - 80);
        text(k_map_x + 230, k_map_y + 330, "选择地图后，这里会绘制排位选点方位图。");
        return;
    }

    text_small(k_map_x + 20, k_map_y + 46, "地图：" + map->name() + "；" + map->feature());
    text_small(k_map_x + 20, k_map_y + 68, "方位：与排位区域选择图一致，左上到右下对应选点图方位。");

    for (const auto& region : map->regions())
    {
        const int left = scale_x(region.x);
        const int top = scale_y(region.y);
        const int right = scale_x(region.x + region.width);
        const int bottom = scale_y(region.y + region.height);
        const int danger = 10 - region.kite_score;
        const COLORREF fill = RGB(230 + danger * 2, 245 - danger * 8, 235 - danger * 10);

        add_click_area(left, top, right, bottom, region.id);
        setfillcolor(fill);
        solidrectangle(left, top, right, bottom);
        setlinecolor(RGB(115, 123, 135));
        rectangle(left, top, right, bottom);

        std::ostringstream label;
        label << region.id << " " << region.name;
        text_small(left + 8, top + 8, label.str());

        std::ostringstream metric;
        metric << "牵制" << region.kite_score << " 开阔" << region.openness;
        text_small(left + 8, top + 30, metric.str());
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
        const int cx = scale_x(region->x + region->width / 2) + index * 24 - 12;
        const int cy = scale_y(region->y + region->height / 2);

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
    int y = k_panel_y + 60;
    for (const auto& data : k_map_data)
    {
        std::ostringstream line;
        line << data.id << " - " << data.name;
        add_click_area(k_panel_x + 18, y - 4, k_panel_x + k_panel_w - 22, y + 24, data.id);
        setlinecolor(RGB(226, 230, 236));
        rectangle(k_panel_x + 18, y - 4, k_panel_x + k_panel_w - 22, y + 24);
        text_small(k_panel_x + 26, y, line.str());
        y += 34;
    }
}

void easyx_ui::draw_survivor_list()
{
    text(k_panel_x + 18, k_panel_y + 18, "求生者编号");
    const int col_width = 155;
    const int row_height = 24;
    const int rows_per_column = 18;
    for (size_t i = 0; i < k_survivor_data.size(); ++i)
    {
        const int col = static_cast<int>(i) / rows_per_column;
        const int row = static_cast<int>(i) % rows_per_column;
        const int x = k_panel_x + 18 + col * col_width;
        const int y = k_panel_y + 58 + row * row_height;

        std::ostringstream line;
        line << k_survivor_data[i].id << "." << k_survivor_data[i].name;
        add_click_area(x - 4, y - 3, x + col_width - 10, y + 20, k_survivor_data[i].id);
        text_small(x, y, line.str());
    }
}

void easyx_ui::draw_hunter_list()
{
    text(k_panel_x + 18, k_panel_y + 18, "监管者编号");
    const int col_width = 225;
    const int row_height = 24;
    const int rows_per_column = 18;
    for (size_t i = 0; i < k_hunter_data.size(); ++i)
    {
        const int col = static_cast<int>(i) / rows_per_column;
        const int row = static_cast<int>(i) % rows_per_column;
        const int x = k_panel_x + 18 + col * col_width;
        const int y = k_panel_y + 58 + row * row_height;

        std::ostringstream line;
        line << k_hunter_data[i].id << "." << k_hunter_data[i].name;
        add_click_area(x - 4, y - 3, x + col_width - 10, y + 20, k_hunter_data[i].id);
        text_small(x, y, line.str());
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

void easyx_ui::draw_prompt(const std::string& prompt, const std::string& error)
{
    const int top = k_panel_y + 780;
    setfillcolor(RGB(255, 255, 255));
    solidrectangle(k_panel_x + 8, top, k_panel_x + k_panel_w - 8, k_panel_y + k_panel_h - 8);

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
    settextstyle(14, 0, "微软雅黑");

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
