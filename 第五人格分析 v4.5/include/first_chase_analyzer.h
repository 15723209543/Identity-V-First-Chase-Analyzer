#pragma once

#include "auxiliary_trait.h"
#include "game_map.h"
#include "hunter.h"
#include "model_params.h"
#include "score_calculator.h"
#include "survivor.h"

#include <string>
#include <vector>

struct analysis_result
{
    game_map selected_map;
    hunter selected_hunter;
    auxiliary_trait selected_auxiliary_trait;
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
        const auxiliary_trait& trait,
        int hunter_region_id,
        const map_position& hunter_position,
        const std::vector<survivor>& survivors) const;

private:
    score_calculator calculator_;
};



