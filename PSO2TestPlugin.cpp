#include "PSO2TestPlugin.h"

#include "InterfaceManager.h"

#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"
#include "d3d9.h"
#include "detours.h"
#include "imgui.h"

#include <memory>
#include <tuple>

using namespace PSO2TestPlugin;

static WNDPROC gameWindowProc = nullptr;
static D3DPRESENT_PARAMETERS options;
static std::unique_ptr<Interface::InterfaceManager> drawManager;

typedef HRESULT(WINAPI* EndScene)(LPDIRECT3DDEVICE9 device);
static EndScene oEndScene = nullptr;

static bool show = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND GetGameWindowHandle() {
    return FindWindow("Phantasy Star Online 2", nullptr);
}

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

std::tuple<IDirect3D9*, IDirect3DDevice9*, HWND> CreateDeviceD3D(HWND hWnd) {
    HWND dummy = CreateWindow("BUTTON", "", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, nullptr, nullptr, nullptr, nullptr);

    IDirect3D9* d3d;
    if (!(d3d = Direct3DCreate9(D3D_SDK_VERSION))) {
        return std::make_tuple(nullptr, nullptr, dummy);
    }

    ZeroMemory(&options, sizeof(options));
    options.Windowed = true;
    options.SwapEffect = D3DSWAPEFFECT_DISCARD;
    options.BackBufferFormat = D3DFMT_UNKNOWN;
    options.hDeviceWindow = dummy;
    options.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    IDirect3DDevice9* device;
    auto status = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &options, &device);
    if (status != D3D_OK) {
        return std::make_tuple(nullptr, nullptr, dummy);
    }

    return std::make_tuple(d3d, device, dummy);
}

void InitImGui(LPDIRECT3DDEVICE9 device) {
    auto gameHWnd = GetGameWindowHandle();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::CaptureMouseFromApp();
    ImGui::GetIO().IniFilename = PSO2TestPlugin::IniFilename;

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
        drawManager->Execute();
    }

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(lpDevice);
}

BOOL WINAPI HookedIsDebuggerPresent() {
    return FALSE;
}

DWORD WINAPI PSO2TestPlugin::Initialize() {
    auto gameHWnd = GetGameWindowHandle();

    IDirect3D9* d3d;
    IDirect3DDevice9* device;
    HWND dummy;
    std::tie(d3d, device, dummy) = CreateDeviceD3D(gameHWnd);
    if (d3d == nullptr || device == nullptr) {
        DestroyWindow(dummy);
        return FALSE;
    }

    auto dxVTable = reinterpret_cast<DWORD_PTR*>(device);
    dxVTable = reinterpret_cast<DWORD_PTR*>(dxVTable[0]);
    oEndScene = reinterpret_cast<EndScene>(dxVTable[42]);

    drawManager = std::make_unique<Interface::InterfaceManager>();
    drawManager->AddHandler([]() {
        ImGui::Begin("a very cool window", &show, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("hm hmm, some very nice text");
        ImGui::End();
    });

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&reinterpret_cast<PVOID&>(oEndScene), reinterpret_cast<PVOID>(HookedEndScene));
    DetourTransactionCommit();

    d3d->Release();
    device->Release();
    DestroyWindow(dummy);

    return TRUE;
}