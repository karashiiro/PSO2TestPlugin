#pragma once

#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <Windows.h>

namespace PSO2TestPlugin::Web {
    constexpr const wchar_t* const UserAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36";

    // Synchronously makes an HTTP(S) request to the specified resource.
    std::vector<char> Request(LPCWSTR serverName, LPCWSTR resource, bool https = false, LPCWSTR method = L"GET");
}