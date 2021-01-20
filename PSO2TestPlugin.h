#pragma once

#include <Windows.h>

namespace PSO2TestPlugin {
    void setHandle(HANDLE newHandle);
    DWORD WINAPI initialize();
    void dispose();
}

extern "C" __declspec(dllexport)
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        auto handle = CreateThread(
                nullptr,
                0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(&PSO2TestPlugin::initialize),
                nullptr,
                0,
                nullptr);
        PSO2TestPlugin::setHandle(handle);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        PSO2TestPlugin::dispose();
    }

    return TRUE;
}
