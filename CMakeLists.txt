cmake_minimum_required(VERSION 3.17)
project(PSO2TestPlugin)

set(CMAKE_CXX_STANDARD 20)

add_library(PSO2TestPlugin SHARED PSO2TestPlugin.cpp InterfaceManager.cpp)

# Dependencies
include(cmake/CPM.cmake)

CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        VERSION 1.80
        DOWNLOAD_ONLY
)
if (imgui_ADDED)
    file(GLOB imgui_SOURCE_FILES ${imgui_SOURCE_DIR}/*.cpp)
    set(imgui_backends ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
    add_library(imgui ${imgui_SOURCE_FILES} ${imgui_backends})
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

CPMAddPackage(
        NAME memory_signature
        GITHUB_REPOSITORY JustasMasiulis/memory_signature
        VERSION 1.0.0
        GIT_HASH 33e7b8a7b941eab8c51324c88fb91ab5904b5650
)
if (memory_signature_ADDED)
    # std::range_error is undefined. It's possible that stdexcept was exposed in a dependency in C++11,
    # and isn't anymore? Or perhaps it's a quirk of the author's compiler.
    execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/patch-memsig.bat WORKING_DIRECTORY ${memory_signature_SOURCE_DIR}/include)
    target_include_directories(memory_signature INTERFACE ${memory_signature_SOURCE_DIR}/include)
endif()

target_link_libraries(PSO2TestPlugin imgui ${Detours_Lib} d3d9 memory_signature)