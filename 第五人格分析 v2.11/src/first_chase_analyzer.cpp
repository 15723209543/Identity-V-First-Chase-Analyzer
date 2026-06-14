#include "first_chase_analyzer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

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

    double best_raw_score = -100000.0;
    for (const auto& survivor_role : survivors)
    {
        score_breakdown score;
        score.character_score = character_score(survivor_role);
        score.hunter_score = hunter_style_score(hunter_role, survivor_role);
        score.map_score = map.position_pressure(
            survivor_role.region_id(),
            survivor_role.position_x(),
            survivor_role.position_y());
        score.distance_score = map.distance_penalty(
            hunter_position,
            { survivor_role.position_x(), survivor_role.position_y() });
        score.counter_score = counter_score(hunter_role, survivor_role);

        // 权重顺序按需求设置：
        // 严重角色克制 > 角色基础指标/监管者风格 > 地图区域。
        score.raw_score = 50.0
            + score.character_score * 1.00
            + score.hunter_score * 0.80
            + score.map_score * 0.55
            + score.distance_score * 0.70
            + score.counter_score * 1.35;
        score.probability = 0.0;
        score.reason = "";
        score.best_action = build_action(map, hunter_role, survivor_role, 0.0);

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
        exp_sum += std::exp((score.raw_score - best_raw_score) / 18.0);
    }

    for (size_t i = 0; i < result.scores.size(); ++i)
    {
        result.scores[i].probability = std::exp((result.scores[i].raw_score - best_raw_score) / 18.0) / exp_sum;
        result.scores[i].best_action = build_action(map, hunter_role, result.survivors[i], result.scores[i].probability);
        result.scores[i].reason = build_reason(map, hunter_role, result.survivors[i], result.scores[i]);
    }

    return result;
}

double first_chase_analyzer::character_score(const survivor& survivor_role) const
{
    const double decode_pressure = (survivor_role.decode_score() - 5.0) * 2.5;
    const double target_pressure = (survivor_role.target_value() - 5.0) * 2.8;
    const double weak_kite_pressure = (5.0 - survivor_role.kite_score()) * 3.8;
    const double assist_pressure = (survivor_role.assist_score() - 5.0) * 0.8;
    const double rescue_pressure = (survivor_role.rescue_score() - 5.0) * 0.4;

    double score = decode_pressure + target_pressure + weak_kite_pressure + assist_pressure + rescue_pressure;

    // 救援位如果本身牵制强，开局硬追往往不划算，所以额外扣分。
    if (survivor_role.rescue_score() >= 8 && survivor_role.kite_score() >= 7)
    {
        score -= 4.0;
    }
    if (survivor_role.has_tag("weak_kiter"))
    {
        score += 5.0;
    }
    return score;
}

double first_chase_analyzer::hunter_style_score(const hunter& hunter_role, const survivor& survivor_role) const
{
    double score = 0.0;
    score += (hunter_role.chase_score() - survivor_role.kite_score()) * 1.15;
    score += (hunter_role.extra_power() - 5.0) * 0.9;

    if (survivor_role.has_tag("decoder"))
    {
        score += (hunter_role.control_score() + hunter_role.info_score() - 10.0) * 0.75;
    }
    if (survivor_role.has_tag("rescue"))
    {
        score += (hunter_role.camp_score() - 6.0) * 0.80;
    }
    if (survivor_role.has_tag("assist"))
    {
        score += (hunter_role.control_score() - 5.0) * 0.45;
    }
    if (survivor_role.kite_score() >= 8 && hunter_role.chase_score() <= 6)
    {
        score -= 7.0;
    }
    if (survivor_role.kite_score() <= 4 && hunter_role.chase_score() >= 8)
    {
        score += 5.0;
    }
    return score;
}

double first_chase_analyzer::counter_score(const hunter& hunter_role, const survivor& survivor_role) const
{
    double score = 0.0;

    if (hunter_role.has_tag("anti_mobility") && survivor_role.has_tag("mobility")) score += 13.0;
    if (hunter_role.has_tag("anti_item") && survivor_role.has_tag("item_dependent")) score += 15.0;
    if (hunter_role.has_tag("anti_heal") && survivor_role.has_tag("healer")) score += 16.0;
    if (hunter_role.has_tag("anti_decoder") && survivor_role.has_tag("decoder")) score += 15.0;
    if (hunter_role.has_tag("area_control") && survivor_role.has_tag("decoder")) score += 8.0;
    if (hunter_role.has_tag("long_range") && survivor_role.has_tag("stunner")) score += 6.0;
    if (hunter_role.has_tag("teleport") && survivor_role.has_tag("stealth")) score += 6.0;
    if (hunter_role.has_tag("anti_loop") && survivor_role.has_tag("kiter")) score += 9.0;

    if (survivor_role.has_tag("portal") && !hunter_role.has_tag("teleport") && !hunter_role.has_tag("anti_mobility"))
    {
        score -= 10.0;
    }
    if (survivor_role.kite_score() >= 8 && hunter_role.chase_score() <= 6)
    {
        score -= 9.0;
    }
    if (survivor_role.rescue_score() >= 8 && hunter_role.camp_score() >= 8)
    {
        score += 6.0;
    }

    // 部分强克制/特殊对局：给出更高权重，满足“克制影响最大”的要求。
    if (hunter_role.name() == "摄影师" && survivor_role.has_tag("decoder")) score += 18.0;
    if (hunter_role.name() == "摄影师" && survivor_role.has_tag("weak_kiter")) score += 10.0;
    if (hunter_role.name() == "疯眼" && survivor_role.has_tag("decoder")) score += 14.0;
    if (hunter_role.name() == "隐士" && survivor_role.has_tag("decoder")) score += 14.0;
    if (hunter_role.name() == "蜡像师" && survivor_role.has_tag("item_dependent")) score += 12.0;
    if (hunter_role.name() == "渔女" && survivor_role.has_tag("kiter")) score += 8.0;
    if (hunter_role.name() == "歌剧演员" && survivor_role.has_tag("weak_kiter")) score += 14.0;
    if (hunter_role.name() == "梦之女巫" && survivor_role.has_tag("decoder")) score += 10.0;
    if (hunter_role.name() == "牙医" && survivor_role.has_tag("airborne")) score += 18.0;
    if (hunter_role.name() == "牙医" && survivor_role.has_tag("healer")) score += 14.0;
    if (hunter_role.name() == "牙医" && survivor_role.has_tag("mobility")) score += 10.0;

    return score;
}

std::string first_chase_analyzer::counter_reason(const hunter& hunter_role, const survivor& survivor_role, double counter_score) const
{
    std::ostringstream out;
    if (counter_score >= 22.0)
    {
        out << "存在严重克制，" << hunter_role.name() << "的技能能明显压低" << survivor_role.name() << "的自保收益";
    }
    else if (counter_score >= 10.0)
    {
        out << "存在中等克制，监管者技能与求生者特性匹配";
    }
    else if (counter_score <= -8.0)
    {
        out << "该求生者机制会反制监管者追击，硬追风险偏高";
    }
    else
    {
        out << "没有明显硬克制，主要看出生点和角色基础价值";
    }
    return out.str();
}

std::string first_chase_analyzer::build_reason(const game_map& map,
    const hunter& hunter_role,
    const survivor& survivor_role,
    const score_breakdown& score) const
{
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    // 监管者输出中第 2 行已经展示概率和各项计算系数。
    // 这里的 reason 只保留文字解释，避免第 3 行“说明”再次重复同一批数据。
    out << counter_reason(hunter_role, survivor_role, score.counter_score) << "；";
    out << "当前位置为" << map.region_name(survivor_role.region_id()) << "，\n"
        << map.region_advice(survivor_role.region_id());
    return out.str();
}

std::string first_chase_analyzer::build_action(const game_map& map,
    const hunter& hunter_role,
    const survivor& survivor_role,
    double probability) const
{
    const map_region_data* region = map.find_region(survivor_role.region_id());
    const bool danger_region = region && region->kite_score <= 4;
    const bool high_probability = probability >= 0.32;

    std::ostringstream out;
    if (high_probability && danger_region)
    {
        out << "不要贪机，马上向强板窗或队友可支援区域转点，并提前保留关键道具。";
    }
    else if (survivor_role.has_tag("decoder"))
    {
        out << "在不暴露脚印的前提下抢破译，一旦监管者靠近就提前离开电机。";
    }
    else if (survivor_role.has_tag("rescue"))
    {
        out << "保存救援道具和血量，不要为了贪牵制把救援节奏送掉。";
    }
    else if (survivor_role.has_tag("assist"))
    {
        out << "观察队友位置，优先保持可支援距离，同时避免在开阔区被先手拿刀。";
    }
    else if (survivor_role.kite_score() >= 8)
    {
        out << "把监管者引向强区消耗时间，但不要被" << hunter_role.name() << "的关键技能封住转点。";
    }
    else
    {
        out << map.region_advice(survivor_role.region_id());
    }
    return out.str();
}
