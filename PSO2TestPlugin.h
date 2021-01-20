#pragma once

#include <Windows.h>

namespace PSO2TestPlugin {
    [[noreturn]] DWORD WINAPI initialize();
}

extern "C" __declspec(dllexport)
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        CreateThread(
                nullptr,
                0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(&PSO2TestPlugin::initialize),
                nullptr,
                0,
                nullptr);
    }
    return TRUE;
}
