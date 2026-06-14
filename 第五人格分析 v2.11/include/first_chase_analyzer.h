#pragma once

#include "game_map.h"
#include "hunter.h"
#include "survivor.h"

#include <string>
#include <vector>

struct score_breakdown
{
    double character_score;
    double hunter_score;
    double map_score;
    double distance_score;
    double counter_score;
    double raw_score;
    double probability;
    std::string reason;
    std::string best_action;
};

struct analysis_result
{
    game_map selected_map;
    hunter selected_hunter;
    int hunter_region_id;
    int hunter_x;
    int hunter_y;
    std::vector<survivor> survivors;
    std::vector<score_breakdown> scores;
    int best_index;
};

// 首追分析器：只负责数学模型，不负责输入输出和绘图。
class first_chase_analyzer
{
public:
    analysis_result analyze(const game_map& map,
        const hunter& hunter_role,
        int hunter_region_id,
        const map_position& hunter_position,
        const std::vector<survivor>& survivors) const;

private:
    double character_score(const survivor& survivor_role) const;
    double hunter_style_score(const hunter& hunter_role, const survivor& survivor_role) const;
    double counter_score(const hunter& hunter_role, const survivor& survivor_role) const;
    std::string counter_reason(const hunter& hunter_role, const survivor& survivor_role, double counter_score) const;
    std::string build_reason(const game_map& map,
        const hunter& hunter_role,
        const survivor& survivor_role,
        const score_breakdown& score) const;
    std::string build_action(const game_map& map,
        const hunter& hunter_role,
        const survivor& survivor_role,
        double probability) const;
};
