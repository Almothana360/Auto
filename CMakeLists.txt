cmake_minimum_required(VERSION 4.3)
project(Auto C)

set(CMAKE_C_STANDARD 99)
include_directories("./libs/include")
include_directories("./headers")
link_directories("./libs/lib")


add_executable(Auto src/main.c
        src/resource_loader.c
        src/ui_manager.c
        src/input_handler.c
        src/cad_context.c
        src/console_cmd.c
        src/gui_panels_raygui.c
        src/gui_panels.c
        src/cad_camera.c
        src/cad_selection.c
        src/cad_tools.c
        src/cad_editor.c
        src/cJSON.c
        src/cad_math.c
        src/layer.c
        src/spatial_tree.c
        src/commands.c
        src/project_io.c
        src/render_utils.c
        src/microui.c)
target_link_options(Auto PRIVATE "SHELL:-Wl,--stack,16777216")
target_link_libraries(Auto raylib m winmm)