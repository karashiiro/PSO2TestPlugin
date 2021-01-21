#include "PSO2TestPlugin.h"

#include <functional>
#include <string>

#include "d3d9.h"
#include "detours.h"
#include "imgui.h"
#include "imgui_impl/imgui_impl_dx9.h"
#include "imgui_impl/imgui_impl_win32.h"

static HWND dummy;
static D3DPRESENT_PARAMETERS options;
static IDirect3D9* d3d;
static IDirect3DDevice9* device;

typedef HRESULT(WINAPI* EndScene)(LPDIRECT3DDEVICE9 device);
EndScene oEndScene = nullptr;

static DWORD_PTR* dxVTable;


enum D3D_FAIL_CAUSE : int {
    D3D_FAIL_NONE,
    D3D_FAIL_VERSION,
    D3D_DEVICE_LOST,
    D3D_DEVICE_INVALID_CALL,
    D3D_DEVICE_NOT_AVAIL,
    D3D_DEVICE_OOM,
    D3D_PARAMS,
};

D3D_FAIL_CAUSE CreateDeviceD3D(HWND hWnd) {
    dummy = CreateWindow("BUTTON", "", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, nullptr, nullptr, nullptr, nullptr);

    if (!(d3d = Direct3DCreate9(D3D_SDK_VERSION)))
        return D3D_FAIL_CAUSE::D3D_FAIL_VERSION;

    ZeroMemory(&options, sizeof(options));
    options.Windowed = true;
    options.SwapEffect = D3DSWAPEFFECT_DISCARD;
    options.BackBufferFormat = D3DFMT_UNKNOWN;
    options.hDeviceWindow = dummy;
    options.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    auto status = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &options, &device);

    if (status != D3D_OK) {
        if (status == D3DERR_DEVICELOST)
            return D3D_FAIL_CAUSE::D3D_DEVICE_LOST;
        else if (status == D3DERR_INVALIDCALL)
            return D3D_FAIL_CAUSE::D3D_DEVICE_INVALID_CALL;
        else if (status == D3DERR_NOTAVAILABLE)
            return D3D_FAIL_CAUSE::D3D_DEVICE_NOT_AVAIL;
        else if (status == D3DERR_OUTOFVIDEOMEMORY)
            return D3D_FAIL_CAUSE::D3D_DEVICE_OOM;
        else
            return D3D_FAIL_CAUSE::D3D_PARAMS;
    }

    return D3D_FAIL_CAUSE::D3D_FAIL_NONE;
}

enum IM_FAIL_CAUSE : int {
    IM_FAIL_NONE,
    IM_FAIL_D3D_VERSION,
    IM_FAIL_D3D_DEVICE_LOST,
    IM_FAIL_D3D_DEVICE_INVALID_CALL,
    IM_FAIL_D3D_DEVICE_NOT_AVAIL,
    IM_FAIL_D3D_DEVICE_OOM,
    IM_FAIL_D3D_PARAMS,
    IM_FAIL_WIN32_INIT,
    IM_FAIL_DX9_INIT,
};

IM_FAIL_CAUSE LoadImGui() {
    auto gameHwnd = FindWindow("Phantasy Star Online 2", nullptr);

    auto didCreateSucceed = CreateDeviceD3D(gameHwnd);
    if (didCreateSucceed != D3D_FAIL_CAUSE::D3D_FAIL_NONE) {
        if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_FAIL_VERSION)
            return IM_FAIL_CAUSE::IM_FAIL_D3D_VERSION;
        else if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_DEVICE_LOST)
            return IM_FAIL_CAUSE::IM_FAIL_D3D_DEVICE_LOST;
        else if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_DEVICE_INVALID_CALL)
            return IM_FAIL_CAUSE::IM_FAIL_D3D_DEVICE_INVALID_CALL;
        else if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_DEVICE_NOT_AVAIL)
            return IM_FAIL_CAUSE::IM_FAIL_D3D_DEVICE_NOT_AVAIL;
        else if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_DEVICE_OOM)
            return IM_FAIL_CAUSE::IM_FAIL_D3D_DEVICE_OOM;
        else
            return IM_FAIL_CAUSE::IM_FAIL_D3D_PARAMS;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::CaptureMouseFromApp();

    if (!ImGui_ImplWin32_Init(gameHwnd)) return IM_FAIL_CAUSE::IM_FAIL_WIN32_INIT;
    if (!ImGui_ImplDX9_Init(device)) return IM_FAIL_CAUSE::IM_FAIL_DX9_INIT;

    return IM_FAIL_CAUSE::IM_FAIL_NONE;
}

static bool imguiInit = false;
HRESULT WINAPI HookedEndScene(LPDIRECT3DDEVICE9 lpDevice) {
    if (!imguiInit) {
        auto gameHwnd = FindWindow("Phantasy Star Online 2", nullptr);
        ImGui_ImplWin32_Init(gameHwnd);
        ImGui_ImplDX9_Init(lpDevice);
        imguiInit = true;
        MessageBox(gameHwnd, "Hooked!", nullptr, 0);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    auto showDemo = true;
    ImGui::ShowDemoWindow(&showDemo);

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(lpDevice);
}

DWORD WINAPI PSO2TestPlugin::Initialize() {
    auto status = LoadImGui();
    if (status != IM_FAIL_CAUSE::IM_FAIL_NONE) {
        constexpr const char* const msgBase = "ImGui load failed with code: ";
        auto msg = std::string(msgBase);
        msg.append(std::to_string(status));
        MessageBox(nullptr, msg.c_str(), nullptr, 0);
    }

    dxVTable = (DWORD_PTR*)device;
    dxVTable = (DWORD_PTR*)dxVTable[0];

    oEndScene = (EndScene)dxVTable[42];

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(LPVOID&)oEndScene, (PBYTE)HookedEndScene);
    DetourTransactionCommit();

    d3d->Release();
    device->Release();
    DestroyWindow(dummy);

    return 1;
}

void PSO2TestPlugin::Dispose() {
    if (oEndScene == nullptr) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(LPVOID&)oEndScene, (PBYTE)HookedEndScene);
    DetourTransactionCommit();
}
