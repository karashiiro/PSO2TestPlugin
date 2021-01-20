#include "PSO2TestPlugin.h"

#include <functional>
#include <string>

#include "d3d9.h"
#include "imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

static bool active = false;
static HANDLE handle;
static IDirect3D9* d3d;
static IDirect3DDevice9* device;

void PSO2TestPlugin::SetHandle(HANDLE newHandle) {
    handle = newHandle;
}

enum D3D_FAIL_CAUSE : int {
    D3D_FAIL_NONE,
    D3D_FAIL_VERSION,
    D3D_FAIL_PARAMS,
};

D3D_FAIL_CAUSE createDeviceD3D(HWND hWnd) {
    D3DPRESENT_PARAMETERS options;

    if (!(d3d = Direct3DCreate9(D3D_SDK_VERSION)))
        return D3D_FAIL_CAUSE::D3D_FAIL_VERSION;

    ZeroMemory(&options, sizeof(options));
    options.Windowed = true;
    options.SwapEffect = D3DSWAPEFFECT_DISCARD;
    options.BackBufferFormat = D3DFMT_UNKNOWN;
    options.EnableAutoDepthStencil = true;
    options.AutoDepthStencilFormat = D3DFMT_D16;
    options.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &options, &device) < 0)
        return D3D_FAIL_CAUSE::D3D_FAIL_PARAMS;

    return D3D_FAIL_CAUSE::D3D_FAIL_NONE;
}

enum IM_FAIL_CAUSE : int {
    IM_FAIL_NONE,
    IM_FAIL_D3D_VERSION,
    IM_FAIL_D3D_PARAMS,
    IM_FAIL_WIN32_INIT,
    IM_FAIL_DX9_INIT,
};

IM_FAIL_CAUSE loadImGui() {
    auto gameHwnd = FindWindowA("Phantasy Star Online 2", nullptr);

    auto didCreateSucceed = createDeviceD3D(gameHwnd);
    if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_FAIL_VERSION) {
        return IM_FAIL_CAUSE::IM_FAIL_D3D_VERSION;
    } else if (didCreateSucceed == D3D_FAIL_CAUSE::D3D_FAIL_PARAMS) {
        return IM_FAIL_CAUSE::IM_FAIL_D3D_PARAMS;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui::GetIO().MouseDrawCursor = true;

    if (!ImGui_ImplWin32_Init(gameHwnd)) return IM_FAIL_CAUSE::IM_FAIL_WIN32_INIT;
    if (!ImGui_ImplDX9_Init(device)) return IM_FAIL_CAUSE::IM_FAIL_DX9_INIT;

    return IM_FAIL_CAUSE::IM_FAIL_NONE;
}

void onDrawUI(const std::function<void()>& drawFunction) {
    while (active) {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        drawFunction();
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
}

DWORD WINAPI PSO2TestPlugin::Initialize() {
    active = true;

    auto status = loadImGui();
    if (status != IM_FAIL_CAUSE::IM_FAIL_NONE) {
        constexpr const char* const msgBase = "ImGui load failed with code: ";
        auto msg = std::string(msgBase);
        msg.append(std::to_string(status));
        MessageBox(nullptr, msg.c_str(), nullptr, 0);
    }

    auto showDemo = true;
    onDrawUI([&]() {
        ImGui::ShowDemoWindow(&showDemo);
    });

    return 1;
}

void PSO2TestPlugin::Dispose() {
    active = false;

    WaitForSingleObject(handle, INFINITE);

    d3d->Release();
    d3d = nullptr;

    device->Release();
    device = nullptr;
}
