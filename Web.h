#pragma once

#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <Windows.h>

namespace PSO2TestPlugin::Web {
    constexpr const wchar_t* const UserAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36";

    /// \brief Synchronously makes an HTTP(S) request to the specified resource.
    /// \param serverName The base URL of the request server. Do not include the schema.
    /// \param resource The path of the desired resource, relative to the base URL.
    /// \param https Whether or not the request should be secured or not.
    /// \param method The request method to use. Request bodies are not currently implemented.
    /// \return The data located at the desired resource.
    std::vector<char> Request(LPCWSTR serverName, LPCWSTR resource, bool https = false, LPCWSTR method = L"GET");
}