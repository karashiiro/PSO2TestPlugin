cmake_minimum_required(VERSION 3.17)
project(PSO2TestPlugin)

set(CMAKE_CXX_STANDARD 20)

add_library(PSO2TestPlugin SHARED PSO2TestPlugin.cpp InterfaceManager.cpp imgui_impl/imgui_impl_win32.cpp imgui_impl/imgui_impl_dx9.cpp)

# Dependencies
include(cmake/CPM.cmake)

CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        VERSION 1.79
        DOWNLOAD_ONLY
)
if (imgui_ADDED)
    file(GLOB imgui_SOURCE_FILES ${imgui_SOURCE_DIR}/*.cpp)
    add_library(imgui ${imgui_SOURCE_FILES})
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

CPMAddPackage(
        NAME Detours
        GITHUB_REPOSITORY microsoft/Detours
        VERSION 4.0.1
        DOWNLOAD_ONLY
)
if (Detours_ADDED)
    # Detours is an nmake library, so we need to force some things in weird ways
    execute_process(COMMAND nmake WORKING_DIRECTORY ${Detours_SOURCE_DIR}/src)
    set(Detours_Lib ${Detours_SOURCE_DIR}/lib.X86/detours.lib)
    target_include_directories(PSO2TestPlugin PUBLIC ${Detours_SOURCE_DIR}/include)
endif()

target_link_libraries(PSO2TestPlugin imgui ${Detours_Lib} d3d9)