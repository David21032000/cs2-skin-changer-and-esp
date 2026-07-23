cmake_minimum_required(VERSION 3.16)
project(cs2-skin-changer
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE IMGUI_SOURCES src/imgui/*.cpp)

add_executable(cs2-skin-changer WIN32
    src/main.cpp
    src/process.cpp
    src/memory.cpp
    src/entity.cpp
    src/skin_changer.cpp
    src/esp.cpp
    src/ui.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(cs2-skin-changer PRIVATE
    src
    src/imgui
)

target_link_libraries(cs2-skin-changer PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    gdi32
)

if(MSVC)
    target_compile_options(cs2-skin-changer PRIVATE /W4 /utf-8)
else()
    target_compile_options(cs2-skin-changer PRIVATE -Wall -Wextra -Wpedantic)
endif()
