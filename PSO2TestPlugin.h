#pragma once

#include <Windows.h>

namespace PSO2TestPlugin {
    constexpr const char* const IniFilename = "PSO2TestPlugin.ini";

    DWORD WINAPI Initialize();
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        CreateThread(
                nullptr,
                0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(&PSO2TestPlugin::Initialize),
                nullptr,
                0,
                nullptr);
    }
    return TRUE;
}
