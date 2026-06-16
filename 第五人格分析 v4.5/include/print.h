#pragma once

#include "first_chase_analyzer.h"

#include <string>
#include <vector>

// print 专门负责把分析结果转换成右侧输出文本和 result 文件。
// 计算类只给数字和原因，print 负责排版，避免计算逻辑里混入大量显示模板。
class print
{
public:
    static std::string build_output_text(const analysis_result& result);
    static std::string save_timestamped_result(const std::string& text);

private:
    static int rounded(double value);
    static int percent_int(double probability);
    static double ability_percent(int raw_score);
    static std::string fixed_2(double value);
    static std::string build_timestamp_file_name();

    // 按中文/英文标点拆分说明文本，让右侧完整分析更容易阅读。
    static std::vector<std::string> wrap_reason_by_punctuation(const std::string& text, int max_chars_per_line);
    static int text_display_length(const std::string& text);
    static bool is_gbk_lead_byte(unsigned char value);
    static bool is_sentence_punctuation(const std::string& text, size_t index);
};
