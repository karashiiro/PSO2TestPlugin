#include "PSO2TestPlugin.h"

#include "resources.h"
#include "Util.h"
#include "Web.h"

#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"
#include "detours.h"
#include "imgui.h"
#include "nlohmann/json.hpp"

using namespace PSO2TestPlugin;

typedef HRESULT(WINAPI* EndScene)(LPDIRECT3DDEVICE9 device);

struct D3D9VTable {
    EndScene EndScene;
};

static WNDPROC gameWindowProc = nullptr;
static D3DPRESENT_PARAMETERS options;
static D3D9VTable* d3d9VTable;
static bool show = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) {
        show = !show;
    }

    if (ImGui::GetIO().WantCaptureMouse && uMsg != WM_MOUSEMOVE) {
        return TRUE;
    }

    return CallWindowProc(gameWindowProc, hWnd, uMsg, wParam, lParam);
}

D3D9VTable* BuildD3D9VTable(HWND hWnd) {
    auto dummy = CreateWindow("BUTTON", "", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, nullptr, nullptr, nullptr, nullptr);

    auto d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == nullptr) {
        return nullptr;
    }

    ZeroMemory(&options, sizeof(options));
    options.Windowed = true;
    options.SwapEffect = D3DSWAPEFFECT_DISCARD;
    options.BackBufferFormat = D3DFMT_UNKNOWN;
    options.hDeviceWindow = dummy;
    options.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    IDirect3DDevice9* device;
    if (const auto status = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &options, &device);
        status != D3D_OK) {
        return nullptr;
    }

    auto dxVTable = reinterpret_cast<DWORD_PTR*>(device);
    dxVTable = reinterpret_cast<DWORD_PTR*>(dxVTable[0]);
    auto oEndScene = reinterpret_cast<EndScene>(dxVTable[42]);

    auto dxVTableCopy = new D3D9VTable{oEndScene};

    d3d->Release();
    device->Release();
    DestroyWindow(dummy);

    return dxVTableCopy;
}

void InitImGui(LPDIRECT3DDEVICE9 device) {
    auto gameHWnd = Util::GetGameWindowHandle();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::CaptureMouseFromApp();
    ImGui::GetIO().IniFilename = PSO2TestPlugin::IniFilename;

    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)open_sans_ttf, static_cast<int>(open_sans_ttf_size), 14, &cfg);

    gameWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(gameHWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookedWndProc)));

    ImGui_ImplWin32_Init(gameHWnd);
    ImGui_ImplDX9_Init(device);
}

HRESULT WINAPI HookedEndScene(LPDIRECT3DDEVICE9 lpDevice) {
    if (gameWindowProc == nullptr) {
        InitImGui(lpDevice);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = ImGui::GetIO().WantCaptureMouse;

    if (show) {
        DrawManager->Execute();
    }

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return d3d9VTable->EndScene(lpDevice);
}

BOOL WINAPI PSO2TestPlugin::Hook() {
    auto gameHWnd = Util::GetGameWindowHandle();
    d3d9VTable = BuildD3D9VTable(gameHWnd);
    if (d3d9VTable == nullptr) {
        return FALSE;
    }

    DrawManager = new Interface::InterfaceManager();

    Initialize();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&reinterpret_cast<PVOID&>(d3d9VTable->EndScene), reinterpret_cast<PVOID>(HookedEndScene));
    DetourTransactionCommit();

    return TRUE;
}

void PSO2TestPlugin::Initialize() {
    auto res = Web::Request(L"xivapi.com", L"/Item/30374", true, L"GET");
    std::string raw(res.begin(), res.end());
    auto data = nlohmann::json::parse(raw);
    auto itemName = data["Name"].get<std::string>();
    static auto text = Util::JoinStrings("the text: ", itemName);

    DrawManager->AddHandler([] {
        ImGui::Begin("a very cool window", &show, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("hm hmm, some very nice text");
        ImGui::Text("%s", text.c_str());
        ImGui::End();
    });
}