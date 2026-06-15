#pragma once

#include "game_map.h"
#include "hunter.h"
#include "model_params.h"
#include "survivor.h"

#include <string>

struct score_breakdown
{
    double character_score;
    double hunter_score;
    double map_score;
    double distance_score;
    double counter_score;
    double raw_score;
    double probability;
    double path_distance_m;
    int nearby_window_count;
    int nearby_pallet_count;
    double nearby_resource_score;
    double special_map_score;
    std::string reason;
    std::string best_action;
};

// score_calculator 集中管理所有和首追分数有关的计算。
// first_chase_analyzer 只负责遍历四名求生者、做概率化和找最高概率目标。
class score_calculator
{
public:
    score_breakdown calculate_base_score(const game_map& map,
        const hunter& hunter_role,
        const map_position& hunter_position,
        const survivor& survivor_role) const;

    void fill_probability_text(const game_map& map,
        const hunter& hunter_role,
        const survivor& survivor_role,
        score_breakdown& score) const;

private:
    double percent_score(int raw_score, const model_params& params) const;
    double character_score(const survivor& survivor_role, const model_params& params) const;
    double hunter_style_score(const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const;
    double special_map_score(const game_map& map, const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const;
    double counter_score(const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const;
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



