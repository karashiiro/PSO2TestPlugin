#pragma once

#define WIN32_LEAN_AND_MEAN
#include "d3d9.h"
#include <Windows.h>

namespace PSO2TestPlugin {
    constexpr const char* const IniFilename = "PSO2TestPlugin.ini";

    /// \brief Initialization function. Should be called before accessing any other namespace object.
    /// \return Whether or not initialization succeeded.
    BOOL WINAPI Initialize();
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&PSO2TestPlugin::Initialize), nullptr, 0, nullptr);
    }
    return TRUE;
}
