#include "model_params.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
    std::string trim(const std::string& value)
    {
        size_t begin = 0;
        while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])) != 0)
        {
            ++begin;
        }

        size_t end = value.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0)
        {
            --end;
        }
        return value.substr(begin, end - begin);
    }
}

model_params model_params_loader::load()
{
    model_params params;
    std::ifstream file("config/model_params.txt");
    if (!file)
    {
        return params;
    }

    std::string line;
    while (std::getline(file, line))
    {
        const size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos)
        {
            line = line.substr(0, comment_pos);
        }

        const size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos)
        {
            continue;
        }

        const std::string key = trim(line.substr(0, equal_pos));
        const std::string raw_value = trim(line.substr(equal_pos + 1));
        if (key.empty() || raw_value.empty())
        {
            continue;
        }

        try
        {
            apply_value(params, key, std::stod(raw_value));
        }
        catch (...)
        {
            // ???????§Õ??????????§µ?????????????????§¹??
        }
    }

    return params;
}

void model_params_loader::apply_value(model_params& params, const std::string& key, double value)
{
    if (key == "raw_base") params.raw_base = value;
    else if (key == "character_weight") params.character_weight = value;
    else if (key == "hunter_style_weight") params.hunter_style_weight = value;
    else if (key == "map_weight") params.map_weight = value;
    else if (key == "distance_weight") params.distance_weight = value;
    else if (key == "counter_weight") params.counter_weight = value;
    else if (key == "softmax_divisor") params.softmax_divisor = value;
    else if (key == "percent_min") params.percent_min = value;
    else if (key == "percent_max") params.percent_max = value;
    else if (key == "percent_middle") params.percent_middle = value;
    else if (key == "decode_weight") params.decode_weight = value;
    else if (key == "target_weight") params.target_weight = value;
    else if (key == "weak_kite_weight") params.weak_kite_weight = value;
    else if (key == "assist_weight") params.assist_weight = value;
    else if (key == "rescue_weight") params.rescue_weight = value;
    else if (key == "strong_rescue_penalty") params.strong_rescue_penalty = value;
    else if (key == "weak_kiter_bonus") params.weak_kiter_bonus = value;
    else if (key == "chase_vs_kite_weight") params.chase_vs_kite_weight = value;
    else if (key == "hunter_extra_power_weight") params.hunter_extra_power_weight = value;
    else if (key == "decoder_control_weight") params.decoder_control_weight = value;
    else if (key == "rescue_camp_weight") params.rescue_camp_weight = value;
    else if (key == "assist_control_weight") params.assist_control_weight = value;
    else if (key == "slow_chase_strong_kiter_penalty") params.slow_chase_strong_kiter_penalty = value;
    else if (key == "fast_chase_weak_kiter_bonus") params.fast_chase_weak_kiter_bonus = value;
    else if (key == "distance_divisor") params.distance_divisor = value;
    else if (key == "window_base") params.window_base = value;
    else if (key == "window_near_bonus") params.window_near_bonus = value;
    else if (key == "pallet_base") params.pallet_base = value;
    else if (key == "pallet_near_bonus") params.pallet_near_bonus = value;
    else if (key == "resource_cap") params.resource_cap = value;
    else if (key == "coaster_score") params.coaster_score = value;
    else if (key == "tram_score") params.tram_score = value;
    else if (key == "rope_score") params.rope_score = value;
    else if (key == "slide_score") params.slide_score = value;
    else if (key == "boat_score") params.boat_score = value;
    else if (key == "subway_score") params.subway_score = value;
    else if (key == "special_resource_counter_bonus") params.special_resource_counter_bonus = value;
    else if (key == "counter_tag_anti_mobility") params.counter_tag_anti_mobility = value;
    else if (key == "counter_tag_anti_item") params.counter_tag_anti_item = value;
    else if (key == "counter_tag_anti_heal") params.counter_tag_anti_heal = value;
    else if (key == "counter_tag_anti_decoder") params.counter_tag_anti_decoder = value;
    else if (key == "counter_tag_area_control_decoder") params.counter_tag_area_control_decoder = value;
    else if (key == "counter_tag_long_range_stunner") params.counter_tag_long_range_stunner = value;
    else if (key == "counter_tag_teleport_stealth") params.counter_tag_teleport_stealth = value;
    else if (key == "counter_tag_anti_loop_kiter") params.counter_tag_anti_loop_kiter = value;
    else if (key == "counter_portal_without_answer") params.counter_portal_without_answer = value;
    else if (key == "counter_strong_kiter_slow_hunter") params.counter_strong_kiter_slow_hunter = value;
    else if (key == "counter_rescue_vs_camp") params.counter_rescue_vs_camp = value;
}

