cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)
find_package(OpenGL REQUIRED)

add_definitions(-DIMGUI_SFML_STATIC)

include_directories(${CMAKE_SOURCE_DIR}/external/imgui)

set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
)

add_executable(fluidSim
    src/main.cpp
    src/imgui-SFML.cpp
    ${IMGUI_SOURCES}
)

target_compile_definitions(fluidSim PRIVATE IMGUI_SFML_STATIC)

target_link_libraries(fluidSim
    sfml-system
    sfml-window
    sfml-graphics
    OpenGL::GL
)

