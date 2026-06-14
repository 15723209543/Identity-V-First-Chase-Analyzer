#include "game_map.h"

#include <cmath>
#include <sstream>

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
