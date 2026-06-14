#include "report_writer.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

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
            << "牵制" << score_level(survivor_role.kite_score()) << "(" << survivor_role.kite_score() << ")，"
            << "辅助" << score_level(survivor_role.assist_score()) << "(" << survivor_role.assist_score() << ")，"
            << "救援" << score_level(survivor_role.rescue_score()) << "(" << survivor_role.rescue_score() << ")" << "\n";
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
            << format_percent(result.scores[i].probability) << "\n";
        out << "计算系数原理：角色系数" << score.character_score
            << "，监管者风格" << score.hunter_score
            << "，地图几何" << score.map_score
            << "，距离" << score.distance_score
            << "，克制" << score.counter_score
            << "，综合分" << score.raw_score << "\n";
        out << "说明：" << score.reason << "\n";
    }

    return out.str();
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

std::string report_writer::build_timestamp_file_name()
{
    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
    localtime_s(&local_time, &now);

    std::ostringstream out;
    out << std::put_time(&local_time, "%Y-%m-%d_%H-%M-%S") << ".txt";
    return out.str();
}
