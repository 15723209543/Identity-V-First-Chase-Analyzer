#include "score_calculator.h"

#include <algorithm>
#include <sstream>

score_breakdown score_calculator::calculate_base_score(const game_map& map,
    const hunter& hunter_role,
    const map_position& hunter_position,
    const survivor& survivor_role) const
{
    score_breakdown score{};
    const model_params params = model_params_loader::load();
    const map_position survivor_position{ survivor_role.position_x(), survivor_role.position_y() };
    const map_resource_detail resource = map.nearby_resource_detail(survivor_position);

    score.character_score = character_score(survivor_role, params);
    score.hunter_score = hunter_style_score(hunter_role, survivor_role, params);
    score.map_score = map.position_pressure(
        survivor_role.region_id(),
        survivor_role.position_x(),
        survivor_role.position_y());
    score.path_distance_m = map.path_distance_m(hunter_position, survivor_position);
    score.distance_score = -(score.path_distance_m / std::max(1.0, params.distance_divisor));
    score.special_map_score = special_map_score(map, hunter_role, survivor_role, params);
    score.counter_score = counter_score(hunter_role, survivor_role, params);
    score.nearby_window_count = resource.window_count;
    score.nearby_pallet_count = resource.pallet_count;
    score.nearby_resource_score = resource.total_score;

    // 权重顺序按需求设置：
    // 严重角色克制 > 角色基础指标/监管者风格 > 地图区域。
    score.raw_score = params.raw_base
        + score.character_score * params.character_weight
        + score.hunter_score * params.hunter_style_weight
        + (score.map_score + score.special_map_score) * params.map_weight
        + score.distance_score * params.distance_weight
        + score.counter_score * params.counter_weight;
    score.probability = 0.0;
    score.reason.clear();
    score.best_action = build_action(map, hunter_role, survivor_role, 0.0);
    return score;
}

void score_calculator::fill_probability_text(const game_map& map,
    const hunter& hunter_role,
    const survivor& survivor_role,
    score_breakdown& score) const
{
    score.best_action = build_action(map, hunter_role, survivor_role, score.probability);
    score.reason = build_reason(map, hunter_role, survivor_role, score);
}

double score_calculator::percent_score(int raw_score, const model_params& params) const
{
    const double clamped = std::max(1.0, std::min(10.0, static_cast<double>(raw_score)));
    return params.percent_min + (clamped - 1.0) * (params.percent_max - params.percent_min) / 9.0;
}

double score_calculator::character_score(const survivor& survivor_role, const model_params& params) const
{
    const double decode = percent_score(survivor_role.decode_score(), params);
    const double kite = percent_score(survivor_role.kite_score(), params);
    const double assist = percent_score(survivor_role.assist_score(), params);
    const double rescue = percent_score(survivor_role.rescue_score(), params);
    const double target = percent_score(survivor_role.target_value(), params);

    const double decode_pressure = (decode - params.percent_middle) * params.decode_weight;
    const double target_pressure = (target - params.percent_middle) * params.target_weight;
    const double weak_kite_pressure = (params.percent_middle - kite) * params.weak_kite_weight;
    const double assist_pressure = (assist - params.percent_middle) * params.assist_weight;
    const double rescue_pressure = (rescue - params.percent_middle) * params.rescue_weight;

    double score = decode_pressure + target_pressure + weak_kite_pressure + assist_pressure + rescue_pressure;

    // 救援位如果本身牵制强，开局硬追往往不划算，所以额外扣分。
    if (survivor_role.rescue_score() >= 8 && survivor_role.kite_score() >= 7)
    {
        score -= params.strong_rescue_penalty;
    }
    if (survivor_role.has_tag("weak_kiter"))
    {
        score += params.weak_kiter_bonus;
    }
    return score;
}

double score_calculator::hunter_style_score(const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const
{
    double score = 0.0;
    score += (percent_score(hunter_role.chase_score(), params) - percent_score(survivor_role.kite_score(), params))
        * params.chase_vs_kite_weight;
    score += (percent_score(hunter_role.extra_power(), params) - params.percent_middle)
        * params.hunter_extra_power_weight;

    if (survivor_role.has_tag("decoder"))
    {
        const double control = percent_score(hunter_role.control_score(), params);
        const double info = percent_score(hunter_role.info_score(), params);
        score += ((control + info) / 2.0 - params.percent_middle) * params.decoder_control_weight;
    }
    if (survivor_role.has_tag("rescue"))
    {
        score += (percent_score(hunter_role.camp_score(), params) - params.percent_middle)
            * params.rescue_camp_weight;
    }
    if (survivor_role.has_tag("assist"))
    {
        score += (percent_score(hunter_role.control_score(), params) - params.percent_middle)
            * params.assist_control_weight;
    }
    if (survivor_role.kite_score() >= 8 && hunter_role.chase_score() <= 6)
    {
        score -= params.slow_chase_strong_kiter_penalty;
    }
    if (survivor_role.kite_score() <= 4 && hunter_role.chase_score() >= 8)
    {
        score += params.fast_chase_weak_kiter_bonus;
    }
    return score;
}

double score_calculator::special_map_score(const game_map& map,
    const hunter& hunter_role,
    const survivor& survivor_role,
    const model_params& params) const
{
    const std::string region_name = map.region_name(survivor_role.region_id());
    double score = 0.0;

    // 特殊地图组件大多帮助求生者转点，因此默认降低监管者首追意愿。
    // 如果监管者有强机动、传送或反位移能力，可以吃掉一部分地形收益。
    if (map.name() == "月亮河公园")
    {
        if (region_name.find("站") != std::string::npos) score += params.coaster_score;
        if (region_name.find("滑梯") != std::string::npos) score += params.slide_score;
        if (region_name.find("桥") != std::string::npos) score += params.rope_score * 0.6;
    }
    if (map.name() == "永眠镇")
    {
        if (region_name.find("站") != std::string::npos) score += params.tram_score;
    }
    if (map.name() == "湖景村")
    {
        if (region_name.find("船") != std::string::npos) score += params.boat_score;
    }
    if (map.name() == "不归林")
    {
        if (region_name.find("树屋") != std::string::npos) score += params.rope_score;
        if (region_name.find("河道") != std::string::npos) score += params.subway_score * 0.6;
    }

    if (score < 0.0
        && (hunter_role.has_tag("anti_mobility")
            || hunter_role.has_tag("teleport")
            || hunter_role.has_tag("fast_chase")))
    {
        score += params.special_resource_counter_bonus;
    }
    return score;
}

double score_calculator::counter_score(const hunter& hunter_role, const survivor& survivor_role, const model_params& params) const
{
    double score = 0.0;

    if (hunter_role.has_tag("anti_mobility") && survivor_role.has_tag("mobility")) score += params.counter_tag_anti_mobility;
    if (hunter_role.has_tag("anti_item") && survivor_role.has_tag("item_dependent")) score += params.counter_tag_anti_item;
    if (hunter_role.has_tag("anti_heal") && survivor_role.has_tag("healer")) score += params.counter_tag_anti_heal;
    if (hunter_role.has_tag("anti_decoder") && survivor_role.has_tag("decoder")) score += params.counter_tag_anti_decoder;
    if (hunter_role.has_tag("area_control") && survivor_role.has_tag("decoder")) score += params.counter_tag_area_control_decoder;
    if (hunter_role.has_tag("long_range") && survivor_role.has_tag("stunner")) score += params.counter_tag_long_range_stunner;
    if (hunter_role.has_tag("teleport") && survivor_role.has_tag("stealth")) score += params.counter_tag_teleport_stealth;
    if (hunter_role.has_tag("anti_loop") && survivor_role.has_tag("kiter")) score += params.counter_tag_anti_loop_kiter;
    if (hunter_role.has_tag("fast_chase") && survivor_role.has_tag("weak_kiter")) score += 8.0;
    if (hunter_role.has_tag("area_control") && survivor_role.has_tag("area_control")) score += 6.0;
    if (hunter_role.has_tag("long_range") && survivor_role.has_tag("airborne")) score += 8.0;
    if (hunter_role.has_tag("info") && survivor_role.has_tag("stealth")) score += 7.0;
    if (hunter_role.has_tag("stun") && survivor_role.has_tag("mobility")) score += 6.0;
    if (hunter_role.has_tag("camp") && survivor_role.has_tag("rescue")) score += 5.0;
    if (hunter_role.has_tag("anti_item") && survivor_role.has_tag("stunner")) score += 5.0;
    if (hunter_role.has_tag("area_control") && survivor_role.has_tag("portal")) score += 4.0;

    if (survivor_role.has_tag("portal") && !hunter_role.has_tag("teleport") && !hunter_role.has_tag("anti_mobility"))
    {
        score += params.counter_portal_without_answer;
    }
    if (survivor_role.kite_score() >= 8 && hunter_role.chase_score() <= 6)
    {
        score += params.counter_strong_kiter_slow_hunter;
    }
    if (survivor_role.rescue_score() >= 8 && hunter_role.camp_score() >= 8)
    {
        score += params.counter_rescue_vs_camp;
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

std::string score_calculator::counter_reason(const hunter& hunter_role, const survivor& survivor_role, double counter_score) const
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

std::string score_calculator::build_reason(const game_map& map,
    const hunter& hunter_role,
    const survivor& survivor_role,
    const score_breakdown& score) const
{
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    // 监管者输出中第 2 行已经展示概率、距离和各项计算系数。
    // 这里的 reason 只保留文字解释，避免第 3 行“说明”再次重复同一批数据。
    out << counter_reason(hunter_role, survivor_role, score.counter_score) << "；";
    if (score.nearby_pallet_count > 0 || score.nearby_window_count > 0)
    {
        out << "附近有" << score.nearby_pallet_count << "块板、"
            << score.nearby_window_count << "扇窗，能提供一定转点空间；";
    }
    if (score.special_map_score < -0.01)
    {
        out << "该区域存在特殊地图转点资源，监管者需要预判对方交互路线；";
    }
    else if (score.special_map_score > 0.01)
    {
        out << "监管者技能可以抵消部分特殊地图资源收益；";
    }
    out << "当前位置为" << map.region_name(survivor_role.region_id()) << "，\n"
        << map.region_advice(survivor_role.region_id());
    return out.str();
}

std::string score_calculator::build_action(const game_map& map,
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



