#pragma once

#include "first_chase_analyzer.h"

#include <string>
#include <vector>

struct map_marker
{
    int region_id;
    std::string label;
    std::string display_name;
    unsigned long color;
    int x = 0;
    int y = 0;
    bool has_position = false;
};

struct click_area
{
    int left;
    int top;
    int right;
    int bottom;
    int value;
};

enum class input_action
{
    select,
    back
};

struct number_input_result
{
    input_action action;
    int value;
};

struct position_input_result
{
    input_action action;
    int region_id;
    int x;
    int y;
};

// EasyX 交互层：左侧画地图，右侧画编号列表和提示。
class easyx_ui
{
public:
    easyx_ui();
    ~easyx_ui();

    void initialize();
    number_input_result ask_map();
    position_input_result ask_position(const game_map& map, const std::string& title, const std::vector<map_marker>& markers, const std::string& back_text);
    number_input_result ask_survivor(const game_map& map, int index, const std::vector<map_marker>& markers);
    number_input_result ask_hunter(const game_map& map, const std::vector<map_marker>& markers);
    void draw_result(const analysis_result& result, const std::string& report_text, const std::string& result_file_name);
    void wait_for_finish();

private:
    number_input_result wait_for_number_click(const std::string& prompt, int min_value, int max_value);
    position_input_result wait_for_position_click(const game_map& map, const std::string& prompt);
    void add_click_area(int left, int top, int right, int bottom, int value);
    void draw_base(const std::string& title);
    void draw_map(const game_map* map, const std::vector<map_marker>& markers);
    void draw_map_list();
    void draw_survivor_list();
    void draw_hunter_list();
    void draw_region_list(const game_map& map);
    void draw_map_information(const game_map& map, const std::string& mode_title);
    void draw_legend(int x, int y);
    void draw_result_page();
    void draw_back_button(const std::string& text);
    void draw_prompt(const std::string& prompt, const std::string& error);
    int draw_wrapped_text(int x, int y, int max_width, int max_y, const std::string& value, int line_height);
    void text(int x, int y, const std::string& value);
    void text_small(int x, int y, const std::string& value);

    std::vector<click_area> click_areas_;
    analysis_result result_cache_;
    std::string result_report_text_;
    std::string result_file_name_;
    int result_scroll_line_ = 0;
    int result_max_scroll_line_ = 0;
};
