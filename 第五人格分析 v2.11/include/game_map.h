#pragma once

#include <string>
#include <vector>

struct map_position
{
    int x;
    int y;
};

struct map_line_data
{
    int x1;
    int y1;
    int x2;
    int y2;
};

struct map_geometry_static_data
{
    int map_id;
    std::string source_file;
    double real_width_m;
    double real_height_m;
    int grid_width;
    int grid_height;
    std::vector<int> wall_cells;
    std::vector<map_line_data> windows;
    std::vector<map_line_data> pallets;
};

// 地图区域坐标使用 0-1000 的相对坐标，EasyX 绘图时再缩放到左侧地图画布。
struct map_region_data
{
    int id;
    std::string name;
    int x;
    int y;
    int width;
    int height;
    int kite_score;   // 求生者在该区域的牵制舒适度，越高越难抓
    int openness;    // 区域开阔程度，越高越容易被监管者快速拿刀
    int decode_value; // 该区域电机/转点价值，越高越值得监管者干扰
    std::string note;
};

struct map_static_data
{
    int id;
    std::string name;
    int width;
    int height;
    std::vector<map_region_data> regions;
    std::string feature;
};

class game_map
{
public:
    game_map();
    explicit game_map(const map_static_data& data);

    int id() const;
    const std::string& name() const;
    const std::string& feature() const;
    const std::vector<map_region_data>& regions() const;
    const map_geometry_static_data* geometry() const;
    const map_region_data* find_region(int region_id) const;
    const map_region_data* find_region_at(int x, int y) const;
    int region_id_at(int x, int y) const;
    std::string region_name(int region_id) const;

    // 地图影响：低牵制区、开阔区、电机价值高的区域会提高首追收益。
    double region_pressure(int survivor_region_id) const;
    double position_pressure(int survivor_region_id, int x, int y) const;

    // 距离惩罚：监管者离求生者越远，开局首追意愿越低。
    double distance_penalty(int hunter_region_id, int survivor_region_id) const;
    double distance_penalty(const map_position& hunter_position, const map_position& survivor_position) const;
    double path_distance_m(const map_position& start, const map_position& end) const;
    double nearby_resource_score(const map_position& position) const;
    bool is_blocked_point(int x, int y) const;

    std::string region_advice(int region_id) const;

private:
    map_static_data data_;
};
