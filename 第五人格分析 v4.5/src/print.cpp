#include "print.h"

#include "model_params.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

std::string print::build_output_text(const analysis_result& result)
{
    std::ostringstream out;

    // 求生者部分保持“三行一人”的格式。
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const survivor& survivor_role = result.survivors[i];
        const score_breakdown& score = result.scores[i];

        out << "求生者编号：" << survivor_role.id()
            << "，求生者名字：" << survivor_role.name()
            << "，出生地：" << result.selected_map.region_name(survivor_role.region_id()) << "\n";
        out << "能力参数：破译" << score_level(survivor_role.decode_score()) << "(" << survivor_role.decode_score() << ")，"
            << fixed_2(ability_percent(survivor_role.decode_score())) << "分，"
            << "牵制" << score_level(survivor_role.kite_score()) << "(" << survivor_role.kite_score() << ")，"
            << fixed_2(ability_percent(survivor_role.kite_score())) << "分，"
            << "辅助" << score_level(survivor_role.assist_score()) << "(" << survivor_role.assist_score() << ")，"
            << fixed_2(ability_percent(survivor_role.assist_score())) << "分，"
            << "救援" << score_level(survivor_role.rescue_score()) << "(" << survivor_role.rescue_score() << ")，"
            << fixed_2(ability_percent(survivor_role.rescue_score())) << "分\n";
        out << "现在应该怎么做：" << score.best_action << "\n";
    }

    out << "\n";

    out << "监管者编号：" << result.selected_hunter.id()
        << "，监管者名字：" << result.selected_hunter.name()
        << "，监管者辅助技能：" << result.selected_auxiliary_trait.name() << "\n";
    out << "能力参数：追击" << score_level(result.selected_hunter.chase_score()) << "(" << result.selected_hunter.chase_score() << ")，"
        << fixed_2(ability_percent(result.selected_hunter.chase_score())) << "分，"
        << "守椅" << score_level(result.selected_hunter.camp_score()) << "(" << result.selected_hunter.camp_score() << ")，"
        << fixed_2(ability_percent(result.selected_hunter.camp_score())) << "分，"
        << "控场" << score_level(result.selected_hunter.control_score()) << "(" << result.selected_hunter.control_score() << ")，"
        << fixed_2(ability_percent(result.selected_hunter.control_score())) << "分，"
        << "信息" << score_level(result.selected_hunter.info_score()) << "(" << result.selected_hunter.info_score() << ")，"
        << fixed_2(ability_percent(result.selected_hunter.info_score())) << "分\n";
    out << "强度分析：" << result.selected_hunter.strength_line()
        << "，辅助技能作用：" << result.selected_auxiliary_trait.effect() << "\n";

    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const survivor& survivor_role = result.survivors[i];
        const score_breakdown& score = result.scores[i];
        const bool first_trait_changed = std::fabs(score.first_hit_time_with_trait_seconds - score.first_hit_time_seconds) >= 0.5;
        const bool down_trait_changed = std::fabs(score.down_time_with_trait_seconds - score.down_time_seconds) >= 0.5;

        out << "求生者：" << survivor_role.name()
            << "，追击意愿：" << fixed_2(score.probability * 100.0) << "%"
            << "，当前距离：" << rounded(score.path_distance_m) << "m\n";
        out << "到达求生者位置时间：" << rounded(score.arrival_time_seconds) << "秒；";
        out << "首刀时间：" << rounded(score.first_hit_time_seconds);
        if (first_trait_changed)
        {
            out << "/" << rounded(score.first_hit_time_with_trait_seconds);
        }
        out << "秒；击倒时间：" << rounded(score.down_time_seconds);
        if (down_trait_changed)
        {
            out << "/" << rounded(score.down_time_with_trait_seconds);
        }
        out << "秒\n";
        out << "计算追击时间原理：克制" << fixed_2(score.counter_score)
            << "，角色强度" << fixed_2(score.character_score)
            << "，监管者风格" << fixed_2(score.hunter_score)
            << "，地图距离" << fixed_2(score.distance_score) << "\n";
        out << "地图追击难度" << fixed_2(score.map_score)
            << "，特殊地图" << fixed_2(score.special_map_score)
            << "，队友配合" << fixed_2(score.teammate_score)
            << "，板窗拖延" << fixed_2(score.nearby_resource_score)
            << "，辅助技能修正" << fixed_2(score.auxiliary_trait_delta_seconds) << "\n";
        const std::vector<std::string> reason_lines = wrap_reason_by_punctuation(score.reason, 50);
        for (size_t line_index = 0; line_index < reason_lines.size(); ++line_index)
        {
            // 说明部分按“50 字以内且尽量以标点结尾”排版，避免右侧输出挤成一整行。
            out << (line_index == 0 ? "说明：" : "")
                << reason_lines[line_index] << "\n";
        }
    }

    return out.str();
}

std::string print::save_timestamped_result(const std::string& text)
{
    const std::filesystem::path result_directory("result");
    std::filesystem::create_directories(result_directory);

    const std::filesystem::path result_path = result_directory / build_timestamp_file_name();
    std::ofstream file(result_path, std::ios::binary);
    file << text;
    return result_path.string();
}

int print::rounded(double value)
{
    return static_cast<int>(std::lround(value));
}

int print::percent_int(double probability)
{
    return rounded(probability * 100.0);
}

double print::ability_percent(int raw_score)
{
    const model_params params = model_params_loader::load();
    const double clamped = std::max(1.0, std::min(10.0, static_cast<double>(raw_score)));
    return params.percent_min + (clamped - 1.0) * (params.percent_max - params.percent_min) / 9.0;
}

std::string print::fixed_2(double value)
{
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    out << value;
    return out.str();
}

std::string print::build_timestamp_file_name()
{
    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
    localtime_s(&local_time, &now);

    std::ostringstream out;
    out << std::put_time(&local_time, "%Y-%m-%d_%H-%M-%S") << ".txt";
    return out.str();
}

std::vector<std::string> print::wrap_reason_by_punctuation(const std::string& text, int max_chars_per_line)
{
    std::vector<std::string> result;
    std::vector<std::string> sentences;
    std::string current_sentence;

    // 先按标点切成句子片段，中文 GBK 字符按两个字节整体移动。
    for (size_t index = 0; index < text.size();)
    {
        if (text[index] == '\r')
        {
            ++index;
            continue;
        }
        if (text[index] == '\n')
        {
            if (!current_sentence.empty())
            {
                sentences.push_back(current_sentence);
                current_sentence.clear();
            }
            ++index;
            continue;
        }

        const unsigned char current = static_cast<unsigned char>(text[index]);
        const size_t step = is_gbk_lead_byte(current) && index + 1 < text.size() ? 2 : 1;
        current_sentence.append(text, index, step);

        if (is_sentence_punctuation(text, index))
        {
            sentences.push_back(current_sentence);
            current_sentence.clear();
        }
        index += step;
    }
    if (!current_sentence.empty())
    {
        sentences.push_back(current_sentence);
    }

    std::string current_line;
    for (const std::string& sentence : sentences)
    {
        const int candidate_length = text_display_length(current_line + sentence);
        if (!current_line.empty() && candidate_length > max_chars_per_line)
        {
            result.push_back(current_line);
            current_line.clear();
        }
        current_line += sentence;
    }
    if (!current_line.empty())
    {
        result.push_back(current_line);
    }
    if (result.empty())
    {
        result.push_back("");
    }
    return result;
}

int print::text_display_length(const std::string& text)
{
    int length = 0;
    for (size_t index = 0; index < text.size();)
    {
        const unsigned char current = static_cast<unsigned char>(text[index]);
        if (current == '\r' || current == '\n')
        {
            ++index;
            continue;
        }
        if (is_gbk_lead_byte(current) && index + 1 < text.size())
        {
            index += 2;
        }
        else
        {
            ++index;
        }
        ++length;
    }
    return length;
}

bool print::is_gbk_lead_byte(unsigned char value)
{
    // GB2312/GBK 中文字符首字节通常落在 0x81-0xFE，计数时需要把双字节当作一个汉字。
    return value >= 0x81 && value <= 0xFE;
}

bool print::is_sentence_punctuation(const std::string& text, size_t index)
{
    const unsigned char current = static_cast<unsigned char>(text[index]);
    if (current < 0x80)
    {
        return current == ',' || current == '.'
            || current == ';' || current == ':'
            || current == '!' || current == '?';
    }

    if (index + 1 >= text.size())
    {
        return false;
    }

    const unsigned char next = static_cast<unsigned char>(text[index + 1]);
    // GBK 常用中文标点：，。；：！？、。
    return (current == 0xA3 && (next == 0xAC || next == 0xAE || next == 0xBB || next == 0xBA || next == 0xA1 || next == 0xBF))
        || (current == 0xA1 && next == 0xA2);
}
