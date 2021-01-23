#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace PSO2TestPlugin {
    constexpr const char* const IniFilename = "PSO2TestPlugin.ini";
    constexpr const wchar_t* const UserAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36";

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
