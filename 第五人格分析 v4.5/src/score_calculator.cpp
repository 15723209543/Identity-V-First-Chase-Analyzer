#include "score_calculator.h"

#include <algorithm>
#include <cmath>
#include <sstream>

score_breakdown score_calculator::calculate_base_score(const game_map& map,
    const hunter& hunter_role,
    const auxiliary_trait& trait,
    const map_position& hunter_position,
    const survivor& survivor_role,
    const std::vector<survivor>& all_survivors) const
{
    // 本函数只负责计算一名求生者的基础追击模型：
    // 1. 读取角色、地图、距离、板窗、队友和克制参数；
    // 2. 把这些参数统一折算为首刀时间和击倒时间；
    // 3. 再按辅助技能开局冷却修正可用时间。
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
    score.teammate_score = teammate_score(map, survivor_role, all_survivors, params);
    score.counter_score = counter_score(hunter_role, survivor_role, params);
    score.nearby_window_count = resource.window_count;
    score.nearby_pallet_count = resource.pallet_count;
    score.nearby_resource_score = resource.total_score;

    score.arrival_time_seconds = score.path_distance_m / std::max(0.1, params.hunter_move_speed_mps);

    // 现在模型以“击倒时间”为最大参考。
    // 旧的角色强度、克制、地图、队友配合不再直接决定概率，而是共同修正首刀和倒地所需秒数。
    // 正向分数代表监管者更好追，所以会减少时间；负向分数代表求生者更好拖，所以会增加时间。
    const double chase_advantage_seconds =
        score.character_score * params.character_time_weight
        + score.hunter_score * params.hunter_time_weight
        + score.counter_score * params.counter_time_weight;
    const double kite_delay_seconds =
        -(score.map_score + score.special_map_score) * params.map_time_weight
        - score.teammate_score * params.teammate_time_weight
        + score.nearby_resource_score * params.resource_time_weight;

    score.first_hit_time_seconds = std::max(params.min_first_hit_seconds,
        score.arrival_time_seconds
        + params.base_first_hit_seconds
        - chase_advantage_seconds * 0.45
        + kite_delay_seconds * 0.55);

    score.down_time_seconds = std::max(params.min_down_seconds,
        score.first_hit_time_seconds
        + params.hit_recovery_seconds
        + params.base_second_hit_seconds
        - chase_advantage_seconds * 0.55
        + kite_delay_seconds * 0.45);

    const double first_trait_delta = auxiliary_trait_first_delta(trait, hunter_role, survivor_role, score);
    score.first_hit_time_with_trait_seconds = score.first_hit_time_seconds;
    if (std::fabs(first_trait_delta) >= 0.01)
    {
        const double candidate_first_hit_time = score.first_hit_time_seconds + first_trait_delta;
        // 辅助技能不能在开局冷却结束前产生追击收益，尤其是闪现不能把首刀时间压到 60 秒以前。
        score.first_hit_time_with_trait_seconds = std::max({
            params.min_first_hit_seconds,
            trait.opening_cooldown_seconds(),
            candidate_first_hit_time
            });
    }

    score.auxiliary_trait_delta_seconds = auxiliary_trait_down_delta(trait, hunter_role, survivor_role, score);
    score.down_time_with_trait_seconds = score.down_time_seconds;
    if (std::fabs(score.auxiliary_trait_delta_seconds) >= 0.01)
    {
        const double candidate_down_time = score.down_time_seconds + score.auxiliary_trait_delta_seconds;
        // 辅助技能修正后的击倒时间同样不能早于开局冷却；冷却没到就不允许“提前使用”。
        score.down_time_with_trait_seconds = std::max({
            params.min_down_seconds,
            trait.opening_cooldown_seconds(),
            candidate_down_time
            });
        score.auxiliary_trait_delta_seconds = score.down_time_with_trait_seconds - score.down_time_seconds;
    }

    // softmax 仍然需要“越大越好”的分数，所以用使用辅助特质后的倒地时间取反。
    score.raw_score = -score.down_time_with_trait_seconds;
    score.probability = 0.0;
    score.reason.clear();
    score.best_action = build_action(map, hunter_role, survivor_role, 0.0);
    return score;
}

void score_calculator::fill_probability_text(const game_map& map,
    const hunter& hunter_role,
    const auxiliary_trait& trait,
    const survivor& survivor_role,
    score_breakdown& score) const
{
    // 概率由 analyzer 统一归一化，本函数只在概率确定后补充行动建议和文字说明。
    score.best_action = build_action(map, hunter_role, survivor_role, score.probability);
    score.reason = build_reason(map, hunter_role, trait, survivor_role, score);
}

double score_calculator::percent_score(int raw_score, const model_params& params) const
{
    // 把 1-10 的原始角色评分转换为百分制，便于拉大强弱差距。
    const double clamped = std::max(1.0, std::min(10.0, static_cast<double>(raw_score)));
    return params.percent_min + (clamped - 1.0) * (params.percent_max - params.percent_min) / 9.0;
}

double score_calculator::character_score(const survivor& survivor_role, const model_params& params) const
{
    // 求生者角色分：修机价值高、牵制弱、目标价值高时，监管者更愿意开局追。
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
    // 监管者风格分：比较监管者追击能力与目标牵制能力，再按控场/守椅等标签微调。
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

double score_calculator::teammate_score(const game_map& map,
    const survivor& survivor_role,
    const std::vector<survivor>& all_survivors,
    const model_params& params) const
{
    double score = 0.0;
    bool near_assist = false;
    bool near_rescue = false;
    bool near_decoder = false;

    const map_position current_position{ survivor_role.position_x(), survivor_role.position_y() };
    for (const survivor& teammate : all_survivors)
    {
        if (teammate.id() == survivor_role.id()
            && teammate.position_x() == survivor_role.position_x()
            && teammate.position_y() == survivor_role.position_y())
        {
            continue;
        }

        const map_position teammate_position{ teammate.position_x(), teammate.position_y() };
        const double distance_m = map.path_distance_m(current_position, teammate_position);
        if (distance_m > params.teammate_near_m)
        {
            continue;
        }

        near_assist = near_assist || teammate.has_tag("assist") || teammate.has_tag("stunner") || teammate.has_tag("healer");
        near_rescue = near_rescue || teammate.has_tag("rescue");
        near_decoder = near_decoder || teammate.has_tag("decoder");
    }

    // 修机位旁边有辅助/救援，说明首追很容易被补状态、挡刀或干扰，收益下降。
    if (survivor_role.has_tag("decoder"))
    {
        if (near_assist)
        {
            score += params.assist_protection_penalty;
        }
        if (near_rescue)
        {
            score += params.rescue_protection_penalty;
        }
        if (!near_assist && !near_rescue)
        {
            score += params.decoder_isolated_bonus;
        }
    }

    // 辅助位或救援位贴着修机位时，监管者也可能先压辅助，拆掉互保结构。
    if (near_decoder && survivor_role.has_tag("assist"))
    {
        score += params.assist_near_decoder_bonus;
    }
    if (near_decoder && survivor_role.has_tag("rescue"))
    {
        score += params.rescue_near_decoder_bonus;
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

double score_calculator::auxiliary_trait_first_delta(const auxiliary_trait& trait,
    const hunter& hunter_role,
    const survivor& survivor_role,
    const score_breakdown& score) const
{
    // 辅助技能的首刀修正：如果首刀发生在开局冷却前，则该技能完全不参与计算。
    if (score.first_hit_time_seconds < trait.opening_cooldown_seconds())
    {
        return 0.0;
    }

    double delta = trait.first_hit_delta_seconds();
    if (trait.has_tag("blink"))
    {
        // 闪现主要解决板窗博弈，当前位置板窗越多，首刀收益越大。
        delta -= std::min(10.0, score.nearby_resource_score * 1.15);
    }
    if (trait.has_tag("anti_control")
        && (survivor_role.has_tag("stunner") || survivor_role.has_tag("airborne") || survivor_role.has_tag("control")))
    {
        delta -= 4.0;
    }
    if (trait.has_tag("far_reposition") && score.path_distance_m >= 55.0)
    {
        delta -= std::min(8.0, score.arrival_time_seconds * 0.25);
    }
    if (trait.has_tag("control_cipher"))
    {
        delta = 0.0;
    }

    (void)hunter_role;
    return delta;
}

double score_calculator::auxiliary_trait_down_delta(const auxiliary_trait& trait,
    const hunter& hunter_role,
    const survivor& survivor_role,
    const score_breakdown& score) const
{
    // 辅助技能的击倒修正：如果正常击倒时间早于开局冷却，则不能假设辅助技能已经可用。
    if (score.down_time_seconds < trait.opening_cooldown_seconds())
    {
        return 0.0;
    }

    double delta = trait.down_delta_seconds();

    // 闪现最适合处理强板窗和高牵制点：板窗越多，瞬间位移补刀的实际收益越明显。
    if (trait.has_tag("blink") && (score.nearby_resource_score >= 3.0 || survivor_role.kite_score() >= 8))
    {
        delta -= std::min(12.0, score.nearby_resource_score * 1.45 + survivor_role.kite_score() * 0.35);
    }

    // 传送和移形解决“开局距离过远”的问题，距离越远，抵消的赶路时间越多。
    if (trait.has_tag("far_reposition") && score.path_distance_m >= 45.0)
    {
        delta -= std::min(14.0, score.arrival_time_seconds * 0.45);
    }

    // 兴奋主要针对强控制位；如果目标没有控制能力，它对直接首追的增益会明显下降。
    if (trait.has_tag("anti_control")
        && (survivor_role.has_tag("stunner") || survivor_role.has_tag("airborne") || survivor_role.has_tag("control")))
    {
        delta -= 8.0;
    }

    // 巡视者咬住缺位移角色时更稳定，面对高机动角色会略打折。
    if (trait.has_tag("lock_target") && !survivor_role.has_tag("mobility"))
    {
        delta -= 2.0;
    }
    if (trait.has_tag("lock_target") && survivor_role.has_tag("mobility"))
    {
        delta += 2.0;
    }

    // 窥视者配合控场型监管者更容易把区域变成慢性消耗区。
    if (trait.has_tag("slow_reveal") && hunter_role.has_tag("area_control"))
    {
        delta -= 2.0;
    }

    // 失常更偏控密码机，不是开局追击技能；追修机位时可回收少量控机收益。
    if (trait.has_tag("control_cipher"))
    {
        delta = 0.0;
    }

    return delta;
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
    const auxiliary_trait& trait,
    const survivor& survivor_role,
    const score_breakdown& score) const
{
    // 说明只输出文字解释；数值已经在“计算追击时间原理”两行展示，避免重复。
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    // 监管者输出中第 2 行已经展示概率、距离和各项计算系数。
    // 这里的 reason 只保留文字解释，避免第 3 行“说明”再次重复同一批数据。
    out << counter_reason(hunter_role, survivor_role, score.counter_score) << "；";
    if (score.nearby_pallet_count > 0 || score.nearby_window_count > 0)
    {
        out << "附近有" << score.nearby_pallet_count << "块板、"
            << score.nearby_window_count << "扇窗，能提供一定转点空间；\n";
    }
    else
    {
        out << "附近板窗资源较少，转点容错偏低；\n";
    }
    if (score.special_map_score < -0.01)
    {
        out << "该区域存在特殊地图转点资源，监管者需要预判对方交互路线；";
    }
    else if (score.special_map_score > 0.01)
    {
        out << "监管者技能可以抵消部分特殊地图资源收益；";
    }
    if (score.teammate_score < -0.01)
    {
        out << "附近队友有互保价值，直接首追可能被补状态或干扰；";
    }
    else if (score.teammate_score > 0.01)
    {
        out << "队友站位让监管者有机会压掉修机与辅助联动；";
    }
    if (score.auxiliary_trait_delta_seconds < -0.01)
    {
        out << trait.name() << "在开局冷却结束后可以压缩追击时间，适合当前目标或距离结构；";
    }
    else if (score.auxiliary_trait_delta_seconds > 0.01)
    {
        out << trait.name() << "更偏控场，对直接击倒帮助有限；";
    }
    else
    {
        out << trait.name() << "当前未改变追击时间，可能是冷却未到或技能本身不直接加速追击；";
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



