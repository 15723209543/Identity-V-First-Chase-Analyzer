#pragma once

#include "auxiliary_trait.h"
#include "game_map.h"
#include "hunter.h"
#include "model_params.h"
#include "survivor.h"

#include <string>
#include <vector>

struct score_breakdown
{
    double character_score;               // 求生者角色价值分：越高越值得监管者首追
    double hunter_score;                  // 监管者风格适配分：越高说明该监管者越适合追这个目标
    double map_score;                     // 出生点基础地图分：越高越偏监管者收益
    double distance_score;                // 路径距离折算分：距离越远分数越低
    double counter_score;                 // 克制关系分：严重克制会明显提高追击收益
    double teammate_score;                // 队友配合分：互保会降低追击收益，拆联动会提高收益
    double raw_score;                     // 概率化前的原始分：当前以击倒时间取反
    double probability;                   // softmax 后的追击意愿概率
    double path_distance_m;               // 监管者到求生者当前位置的寻路距离，单位米
    double arrival_time_seconds;          // 监管者纯赶路到达该位置所需时间，单位秒
    double first_hit_time_seconds;        // 不考虑辅助技能的首刀时间
    double first_hit_time_with_trait_seconds; // 考虑辅助技能后的首刀时间
    double down_time_seconds;             // 不考虑辅助技能的击倒时间
    double down_time_with_trait_seconds;  // 考虑辅助技能后的击倒时间
    double auxiliary_trait_delta_seconds; // 辅助技能对击倒时间的最终修正秒数
    int nearby_window_count;              // 当前位置附近可用窗数量
    int nearby_pallet_count;              // 当前位置附近可用板数量
    double nearby_resource_score;         // 板窗资源合计牵制分
    double special_map_score;             // 过山车、电车、绳索等特殊地图资源分
    std::string reason;                   // 给右侧完整分析使用的文字说明
    std::string best_action;              // 给求生者输出使用的当前建议
};

// score_calculator 集中管理所有和首追分数有关的计算。
// first_chase_analyzer 只负责遍历四名求生者、做概率化和找最高概率目标。
class score_calculator
{
public:
    score_breakdown calculate_base_score(const game_map& map,
        const hunter& hunter_role,
        const auxiliary_trait& trait,
        const map_position& hunter_position,
        const survivor& survivor_role,
        const std::vector<survivor>& all_survivors) const;

    void fill_probability_text(const game_map& map,
        const hunter& hunter_role,
        const auxiliary_trait& trait,
        const survivor& survivor_role,
        score_breakdown& score) const;

private:
    double percent_score(int raw_score, const model_params& params) const;
    double character_score(const survivor& survivor_role, const model_params& params) const;
    double hunter_style_score(const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const;
    double special_map_score(const game_map& map, const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const;
    double teammate_score(const game_map& map, const survivor& survivor_role, const std::vector<survivor>& all_survivors, const model_params& params) const;
    double counter_score(const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const;
    double auxiliary_trait_first_delta(const auxiliary_trait& trait,
        const hunter& hunter_role,
        const survivor& survivor_role,
        const score_breakdown& score) const;
    double auxiliary_trait_down_delta(const auxiliary_trait& trait,
        const hunter& hunter_role,
        const survivor& survivor_role,
        const score_breakdown& score) const;
    std::string counter_reason(const hunter& hunter_role, const survivor& survivor_role, double counter_score) const;
    std::string build_reason(const game_map& map,
        const hunter& hunter_role,
        const auxiliary_trait& trait,
        const survivor& survivor_role,
        const score_breakdown& score) const;
    std::string build_action(const game_map& map,
        const hunter& hunter_role,
        const survivor& survivor_role,
        double probability) const;
};



