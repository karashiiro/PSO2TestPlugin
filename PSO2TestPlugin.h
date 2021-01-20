#pragma once

#include <Windows.h>

namespace PSO2TestPlugin {
    void SetHandle(HANDLE newHandle);
    DWORD WINAPI Initialize();
    void Dispose();
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        auto handle = CreateThread(
                nullptr,
                0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(&PSO2TestPlugin::Initialize),
                nullptr,
                0,
                nullptr);
        PSO2TestPlugin::SetHandle(handle);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        PSO2TestPlugin::Dispose();
    }

    return TRUE;
}
