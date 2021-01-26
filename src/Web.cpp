#include "Web.h"

#include <string>
#include <winhttp.h>

std::vector<char> PSO2TestPlugin::Web::Request(LPCWSTR serverName, LPCWSTR resource, bool https, LPCWSTR method) {
    auto session = WinHttpOpen(UserAgent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0);
    if (session == nullptr) {
        MessageBox(nullptr, ("WinHttpOpen: " + std::to_string(GetLastError())).c_str(), nullptr, 0);
        abort();
    }

    auto port = https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    auto connect = WinHttpConnect(session, serverName, port, 0);
    if (connect == nullptr) {
        MessageBox(nullptr, ("WinHttpConnect: " + std::to_string(GetLastError())).c_str(), nullptr, 0);
        abort();
    }

    auto flags = https ? WINHTTP_FLAG_SECURE : 0;
    auto request = WinHttpOpenRequest(connect, method, resource, nullptr, nullptr, nullptr, flags);
    if (request == nullptr) {
        MessageBox(nullptr, ("WinHttpOpenRequest: " + std::to_string(GetLastError())).c_str(), nullptr, 0);
        abort();
    }

    auto response = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (response == FALSE) {
        MessageBox(nullptr, ("WinHttpSendRequest: " + std::to_string(GetLastError())).c_str(), nullptr, 0);
        abort();
    }

    WinHttpReceiveResponse(request, nullptr);
    std::vector<char> bytes;
    DWORD numBytes = 0;
    do {
        WinHttpQueryDataAvailable(request, &numBytes);
        auto chunk = new char[numBytes];
        ZeroMemory(chunk, numBytes);
        DWORD bytesRead = 0;
        WinHttpReadData(request, chunk, numBytes, &bytesRead);
        bytes.insert(bytes.end(), chunk, chunk + numBytes);
        delete[] chunk;
    } while (numBytes > 0);

    WinHttpCloseHandle(request);
    return bytes;
}