#pragma once

#include <Windows.h>

namespace PSO2TestPlugin {
    DWORD WINAPI Initialize();
    void Dispose();
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
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        PSO2TestPlugin::Dispose();
    }

    return TRUE;
}
