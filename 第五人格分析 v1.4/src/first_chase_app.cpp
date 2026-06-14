#include "first_chase_app.h"

#include "data_repository.h"
#include "easyx_ui.h"
#include "first_chase_analyzer.h"
#include "report_writer.h"

#include <windows.h>

#include <sstream>
#include <vector>

int first_chase_app::run()
{
    easyx_ui ui;
    ui.initialize();

    const int map_id = ui.ask_map();
    const map_static_data* map_data = find_map_data(map_id);
    if (!map_data)
    {
        MessageBoxA(nullptr, "地图编号无效，程序结束。", "错误", MB_OK);
        return 1;
    }

    game_map game_map_obj(*map_data);
    std::vector<map_marker> markers;
    std::vector<survivor> survivors;
    const unsigned long survivor_colors[] = {
        RGB(26, 115, 232),
        RGB(19, 150, 91),
        RGB(245, 124, 0),
        RGB(142, 68, 173)
    };

    // 逐个读取求生者：先输入位置，再输入角色编号。
    for (int i = 1; i <= 4; ++i)
    {
        std::ostringstream region_title;
        region_title << "第" << i << "名求生者：输入所在区域";
        const int region_id = ui.ask_region(game_map_obj, region_title.str(), markers);

        std::ostringstream label;
        label << "S" << i;
        markers.push_back({ region_id, label.str(), "", survivor_colors[(i - 1) % 4] });

        const int survivor_id = ui.ask_survivor(game_map_obj, i, markers);
        const survivor_static_data* survivor_data = find_survivor_data(survivor_id);
        if (!survivor_data)
        {
            MessageBoxA(nullptr, "求生者编号无效，程序结束。", "错误", MB_OK);
            return 1;
        }

        markers.back().display_name = survivor_data->name;
        survivors.push_back(survivor(*survivor_data, region_id));
    }

    const int hunter_region_id = ui.ask_region(game_map_obj, "监管者：输入出生/当前位置区域", markers);
    markers.push_back({ hunter_region_id, "H", "", RGB(210, 35, 35) });

    const int hunter_id = ui.ask_hunter(game_map_obj, markers);
    const hunter_static_data* hunter_data = find_hunter_data(hunter_id);
    if (!hunter_data)
    {
        MessageBoxA(nullptr, "监管者编号无效，程序结束。", "错误", MB_OK);
        return 1;
    }
    hunter hunter_obj(*hunter_data);
    markers.back().display_name = hunter_data->name;

    first_chase_analyzer analyzer;
    analysis_result result = analyzer.analyze(game_map_obj, hunter_obj, hunter_region_id, survivors);

    const std::string output = report_writer::build_output_text(result);
    const std::string result_file_name = report_writer::save_timestamped_result(output);

    ui.draw_result(result, output, result_file_name);
    ui.wait_for_finish();
    return 0;
}
