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
};

struct click_area
{
    int left;
    int top;
    int right;
    int bottom;
    int value;
};

// EasyX 交互层：左侧画地图，右侧画编号列表和提示。
class easyx_ui
{
public:
    easyx_ui();
    ~easyx_ui();

    void initialize();
    int ask_map();
    int ask_region(const game_map& map, const std::string& title, const std::vector<map_marker>& markers);
    int ask_survivor(const game_map& map, int index, const std::vector<map_marker>& markers);
    int ask_hunter(const game_map& map, const std::vector<map_marker>& markers);
    void draw_result(const analysis_result& result, const std::string& report_text, const std::string& result_file_name);
    void wait_for_finish();

private:
    int wait_for_click(const std::string& prompt, int min_value, int max_value);
    void add_click_area(int left, int top, int right, int bottom, int value);
    void draw_base(const std::string& title);
    void draw_map(const game_map* map, const std::vector<map_marker>& markers);
    void draw_map_list();
    void draw_survivor_list();
    void draw_hunter_list();
    void draw_region_list(const game_map& map);
    void draw_prompt(const std::string& prompt, const std::string& error);
    int draw_wrapped_text(int x, int y, int max_width, int max_y, const std::string& value, int line_height);
    void text(int x, int y, const std::string& value);
    void text_small(int x, int y, const std::string& value);

    std::vector<click_area> click_areas_;
};
