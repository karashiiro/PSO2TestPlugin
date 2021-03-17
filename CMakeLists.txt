cmake_minimum_required(VERSION 3.17)
project(PSO2TestPlugin)

set(CMAKE_CXX_STANDARD 20)

add_library(PSO2TestPlugin SHARED src/PSO2TestPlugin.cpp src/InterfaceManager.cpp src/Web.cpp src/Util.cpp)

# Dependencies
include(cmake/CPM.cmake)
include(cmake/EmbedResources.cmake)

CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        VERSION 1.80
        DOWNLOAD_ONLY
)
if (imgui_ADDED)
    file(GLOB imgui_SOURCE_FILES ${imgui_SOURCE_DIR}/*.cpp)
    set(imgui_backends ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
    add_library(imgui ${imgui_SOURCE_FILES} ${imgui_backends})
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

CPMAddPackage(
        NAME Detours
        GITHUB_REPOSITORY microsoft/Detours
        VERSION 4.0.1
)
if (Detours_ADDED)
    # Detours is an nmake library, so we need to prebuild it.
    # We need to do some wack setup to make it work universally, though.
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/BuildDetours.bat WORKING_DIRECTORY ${Detours_SOURCE_DIR}/src)
    set(Detours_Lib ${Detours_SOURCE_DIR}/lib.X64/detours.lib)
    target_include_directories(PSO2TestPlugin PUBLIC ${Detours_SOURCE_DIR}/include)
endif()

CPMAddPackage(
        NAME nlohmann_json
        VERSION 3.9.1
        URL https://github.com/nlohmann/json/releases/download/v3.9.1/include.zip
        URL_HASH SHA256=6bea5877b1541d353bd77bdfbdb2696333ae5ed8f9e8cc22df657192218cad91
)
if (nlohmann_json_ADDED)
    add_library(nlohmann_json INTERFACE IMPORTED)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()

create_resources(${CMAKE_SOURCE_DIR}/resources ${CMAKE_SOURCE_DIR}/src/resources.h)
target_link_libraries(PSO2TestPlugin imgui ${Detours_Lib} d3d11 nlohmann_json winhttp)