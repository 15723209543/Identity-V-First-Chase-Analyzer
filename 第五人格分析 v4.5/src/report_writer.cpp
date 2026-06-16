#include "report_writer.h"
#include "model_params.h"

#include <algorithm>
#include <ctime>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
    bool is_gbk_lead_byte(unsigned char value)
    {
        // GBK 中文通常由两个字节组成。固定字符数换行时，不能把中文拆成半个字。
        return value >= 0x81 && value <= 0xFE;
    }

    size_t next_character_length(const std::string& text, size_t index)
    {
        const unsigned char current = static_cast<unsigned char>(text[index]);
        if (is_gbk_lead_byte(current) && index + 1 < text.size())
        {
            return 2;
        }
        return 1;
    }

    bool is_ascii_digit_character(const std::string& text, size_t index)
    {
        return index < text.size()
            && static_cast<unsigned char>(text[index]) < 0x80
            && std::isdigit(static_cast<unsigned char>(text[index])) != 0;
    }
}

std::string report_writer::build_output_text(const analysis_result& result)
{
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);

    // 求生者部分：每个求生者固定三行。
    // 第 1 行：编号 + 名称 + 出生地。
    // 第 2 行：能力参数。
    // 第 3 行：现在应该怎么做。
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const survivor& survivor_role = result.survivors[i];
        const score_breakdown& score = result.scores[i];
        out << "求生者编号：" << survivor_role.id()
            << "，求生者名字：" << survivor_role.name()
            << "，出生地：" << result.selected_map.region_name(survivor_role.region_id()) << "\n";
        out << "能力参数：破译" << score_level(survivor_role.decode_score()) << "(" << survivor_role.decode_score() << ")，"
            << ability_percent(survivor_role.decode_score()) << "分，"
            << "牵制" << score_level(survivor_role.kite_score()) << "(" << survivor_role.kite_score() << ")，"
            << ability_percent(survivor_role.kite_score()) << "分，"
            << "辅助" << score_level(survivor_role.assist_score()) << "(" << survivor_role.assist_score() << ")，"
            << ability_percent(survivor_role.assist_score()) << "分，"
            << "救援" << score_level(survivor_role.rescue_score()) << "(" << survivor_role.rescue_score() << ")，"
            << ability_percent(survivor_role.rescue_score()) << "分" << "\n";
        out << "现在应该怎么做：" << score.best_action << "\n";
    }

    out << "\n";

    // 监管者部分：先输出监管者身份和能力，再对每个求生者固定三行说明。
    out << "监管者编号：" << result.selected_hunter.id() << "，监管者名字：" << result.selected_hunter.name() << "\n";
    out << result.selected_hunter.strength_line() << "\n";
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const score_breakdown& score = result.scores[i];
        out << "求生者：" << result.survivors[i].name()
            << "，追击意愿概率："
            << format_percent(result.scores[i].probability)
            << "，寻路距离：" << score.path_distance_m << "m\n";
        out << "监管者到达该位置的时间：" << score.arrival_time_seconds
            << "秒，预估首刀时间：" << score.first_hit_time_seconds
            << "秒，预估倒地时间：" << score.down_time_seconds << "秒\n";
        out << "计算追击时间原理：克制" << score.counter_score
            << "，角色强度" << score.character_score
            << "，监管者风格" << score.hunter_score
            << "，地图距离" << score.distance_score
            << "，地图追击难度" << score.map_score
            << "，特殊地图" << score.special_map_score
            << "，队友配合" << score.teammate_score
            << "，板窗拖延" << score.nearby_resource_score
            << "（板" << score.nearby_pallet_count
            << "，窗" << score.nearby_window_count << "）"
            << "，最终以倒地时间" << score.down_time_seconds << "秒排序\n";
        out << "说明：" << score.reason << "\n";
    }

    return wrap_output_text(out.str(), 50);
}

std::string report_writer::save_timestamped_result(const std::string& text)
{
    const std::filesystem::path result_directory("result");
    std::filesystem::create_directories(result_directory);

    const std::filesystem::path result_path = result_directory / build_timestamp_file_name();
    std::ofstream file(result_path, std::ios::binary);
    file << text;
    return result_path.string();
}

std::string report_writer::format_percent(double probability)
{
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    out << probability * 100.0 << "%";
    return out.str();
}

double report_writer::ability_percent(int raw_score)
{
    const model_params params = model_params_loader::load();
    const double clamped = std::max(1.0, std::min(10.0, static_cast<double>(raw_score)));
    return params.percent_min + (clamped - 1.0) * (params.percent_max - params.percent_min) / 9.0;
}

std::string report_writer::wrap_output_text(const std::string& text, int max_chars_per_line)
{
    std::ostringstream wrapped;
    std::string current_line;

    // 逐行处理可以保留原有模板中的段落空行，同时只改变过长行。
    for (size_t i = 0; i <= text.size(); ++i)
    {
        if (i == text.size() || text[i] == '\n')
        {
            wrapped << wrap_single_line(current_line, max_chars_per_line);
            if (i != text.size())
            {
                wrapped << "\n";
            }
            current_line.clear();
        }
        else
        {
            current_line.push_back(text[i]);
        }
    }

    return wrapped.str();
}

std::string report_writer::wrap_single_line(const std::string& line, int max_chars_per_line)
{
    if (max_chars_per_line <= 0)
    {
        return line;
    }

    std::ostringstream wrapped;
    size_t index = 0;
    int char_count = 0;
    bool wrote_any = false;

    while (index < line.size())
    {
        const size_t character_length = next_character_length(line, index);

        // 如果第 50 个字符正好是数字，就先换行，让数字从下一行开始显示。
        if (char_count == max_chars_per_line - 1 && is_ascii_digit_character(line, index))
        {
            if (wrote_any)
            {
                wrapped << "\n";
            }
            char_count = 0;
        }
        else if (char_count >= max_chars_per_line)
        {
            if (wrote_any)
            {
                wrapped << "\n";
            }
            char_count = 0;
        }

        wrapped << line.substr(index, character_length);
        wrote_any = true;
        index += character_length;
        char_count += 1;
    }

    return wrapped.str();
}

std::string report_writer::build_timestamp_file_name()
{
    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
    localtime_s(&local_time, &now);

    std::ostringstream out;
    out << std::put_time(&local_time, "%Y-%m-%d_%H-%M-%S") << ".txt";
    return out.str();
}








