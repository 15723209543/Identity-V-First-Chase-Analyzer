#include "game_map.h"
#include "map_geometry_data.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace
{
    const map_geometry_static_data* find_geometry(int map_id)
    {
        for (const auto& item : k_map_geometry_data)
        {
            if (item.map_id == map_id)
            {
                return &item;
            }
        }
        return nullptr;
    }

    int clamp_int(int value, int low, int high)
    {
        return std::max(low, std::min(value, high));
    }

    int cell_index(const map_geometry_static_data& geometry, int x, int y)
    {
        const int cell_x = clamp_int(x * geometry.grid_width / 1000, 0, geometry.grid_width - 1);
        const int cell_y = clamp_int(y * geometry.grid_height / 1000, 0, geometry.grid_height - 1);
        return cell_y * geometry.grid_width + cell_x;
    }

    map_position cell_center(const map_geometry_static_data& geometry, int index)
    {
        const int cell_x = index % geometry.grid_width;
        const int cell_y = index / geometry.grid_width;
        return {
            (cell_x * 1000 + 500) / geometry.grid_width,
            (cell_y * 1000 + 500) / geometry.grid_height
        };
    }

    bool is_yongmian_forced_open(int x, int y)
    {
        // 永眠镇用户截图中红框位置是道路，绿框位置没有墙。
        // 这个修正用于寻路和点击判定，和 EasyX 显示层保持一致。
        const bool left_lane = x >= 85 && x <= 155 && y >= 115 && y <= 685;
        const bool middle_road = x >= 445 && x <= 590 && y >= 250 && y <= 455;
        return left_lane || middle_road;
    }

    bool should_skip_wall_cell(const map_geometry_static_data& geometry, int grid_x, int grid_y)
    {
        if (geometry.map_id != 4)
        {
            return false;
        }

        const int center_x = (grid_x * 1000 + 500) / geometry.grid_width;
        const int center_y = (grid_y * 1000 + 500) / geometry.grid_height;
        return is_yongmian_forced_open(center_x, center_y);
    }

    std::vector<unsigned char> build_blocked_grid(const map_geometry_static_data& geometry)
    {
        std::vector<unsigned char> blocked(geometry.grid_width * geometry.grid_height, 0);
        for (const int index : geometry.wall_cells)
        {
            if (index >= 0 && index < static_cast<int>(blocked.size()))
            {
                const int grid_x = index % geometry.grid_width;
                const int grid_y = index / geometry.grid_width;
                if (!should_skip_wall_cell(geometry, grid_x, grid_y))
                {
                    blocked[index] = 1;
                }
            }
        }
        return blocked;
    }

    int nearest_free_cell(const map_geometry_static_data& geometry,
        const std::vector<unsigned char>& blocked,
        int start_index)
    {
        if (start_index >= 0
            && start_index < static_cast<int>(blocked.size())
            && !blocked[start_index])
        {
            return start_index;
        }

        std::vector<unsigned char> seen(blocked.size(), 0);
        std::queue<int> queue;
        queue.push(start_index);
        if (start_index >= 0 && start_index < static_cast<int>(seen.size()))
        {
            seen[start_index] = 1;
        }

        while (!queue.empty())
        {
            const int current = queue.front();
            queue.pop();
            const int current_x = current % geometry.grid_width;
            const int current_y = current / geometry.grid_width;

            for (const auto& delta : { map_position{1, 0}, map_position{-1, 0}, map_position{0, 1}, map_position{0, -1} })
            {
                const int next_x = current_x + delta.x;
                const int next_y = current_y + delta.y;
                if (next_x < 0 || next_x >= geometry.grid_width || next_y < 0 || next_y >= geometry.grid_height)
                {
                    continue;
                }

                const int next = next_y * geometry.grid_width + next_x;
                if (seen[next])
                {
                    continue;
                }
                if (!blocked[next])
                {
                    return next;
                }
                seen[next] = 1;
                queue.push(next);
            }
        }
        return start_index;
    }

    double distance_to_segment(const map_position& position, const map_line_data& line)
    {
        const double px = static_cast<double>(position.x);
        const double py = static_cast<double>(position.y);
        const double ax = static_cast<double>(line.x1);
        const double ay = static_cast<double>(line.y1);
        const double bx = static_cast<double>(line.x2);
        const double by = static_cast<double>(line.y2);
        const double vx = bx - ax;
        const double vy = by - ay;
        const double length_sq = vx * vx + vy * vy;

        if (length_sq <= 0.01)
        {
            return std::sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
        }

        double t = ((px - ax) * vx + (py - ay) * vy) / length_sq;
        t = std::max(0.0, std::min(1.0, t));
        const double cx = ax + t * vx;
        const double cy = ay + t * vy;
        return std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
    }

    bool line_same_position(const map_line_data& a, const map_line_data& b, int max_center_distance)
    {
        const int ax = (a.x1 + a.x2) / 2;
        const int ay = (a.y1 + a.y2) / 2;
        const int bx = (b.x1 + b.x2) / 2;
        const int by = (b.y1 + b.y2) / 2;
        const int dx = ax - bx;
        const int dy = ay - by;
        return dx * dx + dy * dy <= max_center_distance * max_center_distance;
    }

    std::vector<map_line_data> unique_lines(const std::vector<map_line_data>& lines, int max_center_distance)
    {
        // 图片识别会把同一扇窗或同一块板拆出多条很近的线。
        // 算法评分时也要合并，避免同一位置被重复加成。
        std::vector<map_line_data> result;
        for (const auto& line_data : lines)
        {
            bool duplicated = false;
            for (const auto& kept : result)
            {
                if (line_same_position(line_data, kept, max_center_distance))
                {
                    duplicated = true;
                    break;
                }
            }
            if (!duplicated)
            {
                result.push_back(line_data);
            }
        }
        return result;
    }
}

game_map::game_map()
    : data_{}
{
}

game_map::game_map(const map_static_data& data)
    : data_(data)
{
}

int game_map::id() const { return data_.id; }
const std::string& game_map::name() const { return data_.name; }
const std::string& game_map::feature() const { return data_.feature; }
const std::vector<map_region_data>& game_map::regions() const { return data_.regions; }
const map_geometry_static_data* game_map::geometry() const { return find_geometry(data_.id); }

const map_region_data* game_map::find_region(int region_id) const
{
    for (const auto& region : data_.regions)
    {
        if (region.id == region_id)
        {
            return &region;
        }
    }
    return nullptr;
}

const map_region_data* game_map::find_region_at(int x, int y) const
{
    for (const auto& region : data_.regions)
    {
        if (x >= region.x
            && x <= region.x + region.width
            && y >= region.y
            && y <= region.y + region.height)
        {
            return &region;
        }
    }

    const map_region_data* best_region = nullptr;
    double best_distance = std::numeric_limits<double>::max();
    for (const auto& region : data_.regions)
    {
        const double center_x = region.x + region.width / 2.0;
        const double center_y = region.y + region.height / 2.0;
        const double distance = (center_x - x) * (center_x - x) + (center_y - y) * (center_y - y);
        if (distance < best_distance)
        {
            best_distance = distance;
            best_region = &region;
        }
    }
    return best_region;
}

int game_map::region_id_at(int x, int y) const
{
    const map_region_data* region = find_region_at(x, y);
    return region ? region->id : 0;
}

std::string game_map::region_name(int region_id) const
{
    const map_region_data* region = find_region(region_id);
    return region ? region->name : "未知区域";
}

double game_map::region_pressure(int survivor_region_id) const
{
    const map_region_data* region = find_region(survivor_region_id);
    if (!region)
    {
        return 0.0;
    }

    // kite_score 越低表示越好抓；openness 和 decode_value 越高表示越值得压。
    const double weak_kite_score = (10.0 - region->kite_score) * 1.8;
    const double open = (region->openness - 5.0) * 1.0;
    const double decode = (region->decode_value - 5.0) * 0.9;
    return weak_kite_score + open + decode;
}

double game_map::position_pressure(int survivor_region_id, int x, int y) const
{
    const double base_pressure = region_pressure(survivor_region_id);
    const double resource_defense = nearby_resource_score({ x, y });
    return base_pressure - resource_defense * 1.7;
}

double game_map::distance_penalty(int hunter_region_id, int survivor_region_id) const
{
    const map_region_data* hunter_region = find_region(hunter_region_id);
    const map_region_data* survivor_region = find_region(survivor_region_id);
    if (!hunter_region || !survivor_region)
    {
        return 0.0;
    }

    const double hx = hunter_region->x + hunter_region->width / 2.0;
    const double hy = hunter_region->y + hunter_region->height / 2.0;
    const double sx = survivor_region->x + survivor_region->width / 2.0;
    const double sy = survivor_region->y + survivor_region->height / 2.0;
    const double distance = std::sqrt((hx - sx) * (hx - sx) + (hy - sy) * (hy - sy));

    // 1000 相对坐标下，500 左右已是明显跨图距离。
    return -(distance / 100.0) * 1.25;
}

double game_map::distance_penalty(const map_position& hunter_position, const map_position& survivor_position) const
{
    const double distance_m = path_distance_m(hunter_position, survivor_position);
    return -(distance_m / 10.0);
}

double game_map::path_distance_m(const map_position& start, const map_position& end) const
{
    const map_geometry_static_data* geometry_data = geometry();
    if (!geometry_data)
    {
        const double dx = (start.x - end.x) / 1000.0 * 150.0;
        const double dy = (start.y - end.y) / 1000.0 * 150.0;
        return std::sqrt(dx * dx + dy * dy);
    }

    const auto blocked = build_blocked_grid(*geometry_data);
    int start_index = cell_index(*geometry_data, start.x, start.y);
    int end_index = cell_index(*geometry_data, end.x, end.y);
    start_index = nearest_free_cell(*geometry_data, blocked, start_index);
    end_index = nearest_free_cell(*geometry_data, blocked, end_index);

    const int total = geometry_data->grid_width * geometry_data->grid_height;
    std::vector<double> distance(total, std::numeric_limits<double>::infinity());
    std::vector<unsigned char> visited(total, 0);

    using queue_item = std::pair<double, int>;
    std::priority_queue<queue_item, std::vector<queue_item>, std::greater<queue_item>> queue;
    distance[start_index] = 0.0;
    queue.push({ 0.0, start_index });

    const double cell_width_m = geometry_data->real_width_m / geometry_data->grid_width;
    const double cell_height_m = geometry_data->real_height_m / geometry_data->grid_height;
    const int dx_values[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };
    const int dy_values[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };

    while (!queue.empty())
    {
        const auto [current_distance, current] = queue.top();
        queue.pop();
        if (visited[current])
        {
            continue;
        }
        visited[current] = 1;
        if (current == end_index)
        {
            return current_distance;
        }

        const int current_x = current % geometry_data->grid_width;
        const int current_y = current / geometry_data->grid_width;
        for (int i = 0; i < 8; ++i)
        {
            const int next_x = current_x + dx_values[i];
            const int next_y = current_y + dy_values[i];
            if (next_x < 0 || next_x >= geometry_data->grid_width || next_y < 0 || next_y >= geometry_data->grid_height)
            {
                continue;
            }

            const int next = next_y * geometry_data->grid_width + next_x;
            if (blocked[next])
            {
                continue;
            }
            if (dx_values[i] != 0 && dy_values[i] != 0)
            {
                const int side_a = current_y * geometry_data->grid_width + next_x;
                const int side_b = next_y * geometry_data->grid_width + current_x;
                if (blocked[side_a] || blocked[side_b])
                {
                    continue;
                }
            }

            const double step = std::sqrt(
                (dx_values[i] * cell_width_m) * (dx_values[i] * cell_width_m)
                + (dy_values[i] * cell_height_m) * (dy_values[i] * cell_height_m));
            const double candidate = current_distance + step;
            if (candidate < distance[next])
            {
                distance[next] = candidate;
                queue.push({ candidate, next });
            }
        }
    }

    const double dx = (start.x - end.x) / 1000.0 * geometry_data->real_width_m;
    const double dy = (start.y - end.y) / 1000.0 * geometry_data->real_height_m;
    return std::sqrt(dx * dx + dy * dy) * 1.35;
}

double game_map::nearby_resource_score(const map_position& position) const
{
    const map_geometry_static_data* geometry_data = geometry();
    if (!geometry_data)
    {
        return 0.0;
    }

    double score = 0.0;
    const auto windows = unique_lines(geometry_data->windows, 18);
    for (const auto& window : windows)
    {
        const double distance = distance_to_segment(position, window);
        if (distance <= 80.0)
        {
            score += (80.0 - distance) / 80.0 * 1.25;
        }
    }
    const auto pallets = unique_lines(geometry_data->pallets, 18);
    for (const auto& pallet : pallets)
    {
        const double distance = distance_to_segment(position, pallet);
        if (distance <= 90.0)
        {
            score += (90.0 - distance) / 90.0 * 1.0;
        }
    }
    return std::min(score, 6.0);
}

bool game_map::is_blocked_point(int x, int y) const
{
    const map_geometry_static_data* geometry_data = geometry();
    if (!geometry_data)
    {
        return false;
    }

    const auto blocked = build_blocked_grid(*geometry_data);
    const int index = cell_index(*geometry_data, x, y);
    return index >= 0 && index < static_cast<int>(blocked.size()) && blocked[index];
}

std::string game_map::region_advice(int region_id) const
{
    const map_region_data* region = find_region(region_id);
    if (!region)
    {
        return "先确认出生点，再向安全区转移。";
    }

    std::ostringstream out;
    if (region->kite_score >= 8)
    {
        out << "利用" << region->name << "的强板窗拖时间，不要急着离开强区。";
    }
    else if (region->kite_score <= 4)
    {
        out << "尽快离开" << region->name << "，优先向高墙、板窗或队友可支援区域转点。";
    }
    else
    {
        out << "先消耗" << region->name << "资源，吃刀前规划下一段转点路线。";
    }
    return out.str();
}
