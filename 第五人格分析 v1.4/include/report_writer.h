#pragma once

#include "first_chase_analyzer.h"

#include <string>

// 输出器：负责把分析结果组织成题目要求的固定文本格式。
class report_writer
{
public:
    static std::string build_output_text(const analysis_result& result);
    static std::string save_timestamped_result(const std::string& text);

private:
    static std::string format_percent(double probability);
    static std::string build_timestamp_file_name();
};
