#include "report_writer.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

std::string report_writer::build_output_text(const analysis_result& result)
{
    std::ostringstream out;

    // 前 12 行：每个求生者固定 3 行。
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        const survivor& survivor_role = result.survivors[i];
        const score_breakdown& score = result.scores[i];
        out << "求生者编号：" << survivor_role.id() << "，求生者名字：" << survivor_role.name() << "\n";
        out << survivor_role.metric_line(result.selected_map.region_name(survivor_role.region_id())) << "\n";
        out << survivor_role.advice(score.probability, score.best_action) << "\n";
    }

    out << "\n";

    // 监管者信息：1 行身份，1 行强度，4 行首追概率与原因。
    out << "监管者编号：" << result.selected_hunter.id() << "，监管者名字：" << result.selected_hunter.name() << "\n";
    out << result.selected_hunter.strength_line() << "\n";
    for (size_t i = 0; i < result.survivors.size(); ++i)
    {
        out << "对求生者" << (i + 1) << "（"
            << result.survivors[i].name() << "）的首追概率："
            << format_percent(result.scores[i].probability)
            << "，计算原因：" << result.scores[i].reason << "\n";
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
