cmake_minimum_required(VERSION 3.22)

project(Chip8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages
find_package(OpenGL REQUIRED)

# Include Dear ImGui
include(AddDearImgui)

add_executable(Chip8 main.cpp)

target_link_libraries(Chip8 PRIVATE 
    dear_imgui
    OpenGL::GL
)
