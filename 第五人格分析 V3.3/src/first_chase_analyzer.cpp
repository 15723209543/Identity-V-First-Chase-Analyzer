#include "first_chase_analyzer.h"

#include <algorithm>
#include <cmath>

analysis_result first_chase_analyzer::analyze(const game_map& map,
    const hunter& hunter_role,
    int hunter_region_id,
    const map_position& hunter_position,
    const std::vector<survivor>& survivors) const
{
    analysis_result result;
    result.selected_map = map;
    result.selected_hunter = hunter_role;
    result.hunter_region_id = hunter_region_id;
    result.hunter_x = hunter_position.x;
    result.hunter_y = hunter_position.y;
    result.survivors = survivors;
    result.best_index = 0;

    const model_params params = model_params_loader::load();
    double best_raw_score = -100000.0;
    for (const auto& survivor_role : survivors)
    {
        score_breakdown score = calculator_.calculate_base_score(map, hunter_role, hunter_position, survivor_role);
        result.scores.push_back(score);
        if (score.raw_score > best_raw_score)
        {
            best_raw_score = score.raw_score;
            result.best_index = static_cast<int>(result.scores.size()) - 1;
        }
    }

    // softmax 概率化，避免只做线性归一导致小差距被夸大。
    double exp_sum = 0.0;
    for (const auto& score : result.scores)
    {
        exp_sum += std::exp((score.raw_score - best_raw_score) / std::max(1.0, params.softmax_divisor));
    }

    for (size_t i = 0; i < result.scores.size(); ++i)
    {
        result.scores[i].probability = std::exp((result.scores[i].raw_score - best_raw_score) / std::max(1.0, params.softmax_divisor)) / exp_sum;
        calculator_.fill_probability_text(map, hunter_role, result.survivors[i], result.scores[i]);
    }

    return result;
}



