#pragma once

#include <string>

struct model_params
{
    // 总公式权重：控制不同模块对最终首追分数的影响。
    double raw_base = 50.0;
    double character_weight = 1.35;
    double hunter_style_weight = 0.95;
    double map_weight = 0.62;
    double distance_weight = 0.78;
    double counter_weight = 1.55;
    double softmax_divisor = 16.0;

    // 百分制折算：把原始 1-10 分拉伸到 60-100 分，扩大角色差距。
    double percent_min = 60.0;
    double percent_max = 100.0;
    double percent_middle = 80.0;

    // 求生者指标权重：破译、首抓价值和弱牵制会提高首追收益。
    double decode_weight = 0.72;
    double target_weight = 0.82;
    double weak_kite_weight = 1.05;
    double assist_weight = 0.28;
    double rescue_weight = 0.14;
    double strong_rescue_penalty = 7.0;
    double weak_kiter_bonus = 8.0;

    // 监管者风格权重：用于判断监管者能力是否适合追当前求生者。
    double chase_vs_kite_weight = 0.42;
    double hunter_extra_power_weight = 0.34;
    double decoder_control_weight = 0.26;
    double rescue_camp_weight = 0.24;
    double assist_control_weight = 0.18;
    double slow_chase_strong_kiter_penalty = 10.0;
    double fast_chase_weak_kiter_bonus = 8.0;

    // 地图几何和板窗资源权重：距离越远、板窗越多，首追收益越低。
    double distance_divisor = 9.0;
    double window_base = 1.35;
    double window_near_bonus = 0.55;
    double pallet_base = 1.00;
    double pallet_near_bonus = 0.45;
    double resource_cap = 8.0;

    // 特殊地图资源：过山车、电车、荡绳等通常帮助求生者快速转点。
    double coaster_score = -8.0;
    double tram_score = -7.0;
    double rope_score = -7.0;
    double slide_score = -4.0;
    double boat_score = -5.0;
    double subway_score = -5.0;
    double special_resource_counter_bonus = 4.0;

    // 通用克制标签：用标签扩展克制关系，避免只靠少量硬编码角色名。
    double counter_tag_anti_mobility = 16.0;
    double counter_tag_anti_item = 17.0;
    double counter_tag_anti_heal = 18.0;
    double counter_tag_anti_decoder = 17.0;
    double counter_tag_area_control_decoder = 10.0;
    double counter_tag_long_range_stunner = 8.0;
    double counter_tag_teleport_stealth = 8.0;
    double counter_tag_anti_loop_kiter = 11.0;
    double counter_portal_without_answer = -13.0;
    double counter_strong_kiter_slow_hunter = -12.0;
    double counter_rescue_vs_camp = 8.0;
};

class model_params_loader
{
public:
    // 每次分析前调用，保证修改 txt 后不用重新编译程序。
    static model_params load();

private:
    // 将 txt 中的 key=value 写回结构体；未知 key 会被忽略，方便以后扩展。
    static void apply_value(model_params& params, const std::string& key, double value);
};

