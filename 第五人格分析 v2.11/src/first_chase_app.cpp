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

    while (true)
    {
        const number_input_result map_input = ui.ask_map();
        const map_static_data* map_data = find_map_data(map_input.value);
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

        bool restart_map_selection = false;
        while (true)
        {
            while (survivors.size() < 4)
            {
                const int index = static_cast<int>(survivors.size()) + 1;
                std::ostringstream region_title;
                region_title << "第" << index << "名求生者：点击所在位置";
                const std::string back_text = survivors.empty() ? "返回选地图" : "撤销上一名";
                const position_input_result position = ui.ask_position(game_map_obj, region_title.str(), markers, back_text);

                if (position.action == input_action::back)
                {
                    if (survivors.empty())
                    {
                        restart_map_selection = true;
                        break;
                    }

                    survivors.pop_back();
                    markers.pop_back();
                    continue;
                }

                std::ostringstream label;
                label << "S" << index;
                markers.push_back({
                    position.region_id,
                    label.str(),
                    "",
                    survivor_colors[(index - 1) % 4],
                    position.x,
                    position.y,
                    true
                    });

                const number_input_result survivor_input = ui.ask_survivor(game_map_obj, index, markers);
                if (survivor_input.action == input_action::back)
                {
                    markers.pop_back();
                    continue;
                }

                const survivor_static_data* survivor_data = find_survivor_data(survivor_input.value);
                if (!survivor_data)
                {
                    MessageBoxA(nullptr, "求生者编号无效，程序结束。", "错误", MB_OK);
                    return 1;
                }

                markers.back().display_name = survivor_data->name;
                survivors.push_back(survivor(*survivor_data, position.region_id, position.x, position.y));
            }

            if (restart_map_selection)
            {
                break;
            }

            const position_input_result hunter_position = ui.ask_position(
                game_map_obj,
                "监管者：点击出生/当前位置",
                markers,
                "撤销上一名");
            if (hunter_position.action == input_action::back)
            {
                if (!survivors.empty())
                {
                    survivors.pop_back();
                    markers.pop_back();
                }
                continue;
            }

            markers.push_back({
                hunter_position.region_id,
                "H",
                "",
                RGB(210, 35, 35),
                hunter_position.x,
                hunter_position.y,
                true
                });

            const number_input_result hunter_input = ui.ask_hunter(game_map_obj, markers);
            if (hunter_input.action == input_action::back)
            {
                markers.pop_back();
                continue;
            }

            const hunter_static_data* hunter_data = find_hunter_data(hunter_input.value);
            if (!hunter_data)
            {
                MessageBoxA(nullptr, "监管者编号无效，程序结束。", "错误", MB_OK);
                return 1;
            }
            hunter hunter_obj(*hunter_data);
            markers.back().display_name = hunter_data->name;

            first_chase_analyzer analyzer;
            analysis_result result = analyzer.analyze(
                game_map_obj,
                hunter_obj,
                hunter_position.region_id,
                { hunter_position.x, hunter_position.y },
                survivors);

            const std::string output = report_writer::build_output_text(result);
            const std::string result_file_name = report_writer::save_timestamped_result(output);

            ui.draw_result(result, output, result_file_name);
            ui.wait_for_finish();
            return 0;
        }
    }
}
