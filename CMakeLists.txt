cmake_minimum_required(VERSION 3.17)
project(PSO2TestPlugin)

set(CMAKE_CXX_STANDARD 20)

add_library(PSO2TestPlugin SHARED PSO2TestPlugin.cpp PSO2TestPlugin.h imgui_impl/imgui_impl_win32.h imgui_impl/imgui_impl_win32.cpp imgui_impl/imgui_impl_dx9.h imgui_impl/imgui_impl_dx9.cpp)

# Dependencies
include(cmake/CPM.cmake)

CPMAddPackage(
        NAME imgui_impl
        GITHUB_REPOSITORY ocornut/imgui
        VERSION 1.79
)
if (imgui_ADDED)
    file(GLOB imgui_SOURCE_FILES ${imgui_SOURCE_DIR}/*.cpp)
    add_library(imgui_impl ${imgui_SOURCE_FILES})
    target_include_directories(imgui_impl PUBLIC ${imgui_SOURCE_DIR})
endif()

# I'd like to get this working on g++, but it screams about Windows libraries even with the hint path set.
target_link_libraries(PSO2TestPlugin imgui_impl d3d9)