#include "PSO2TestPlugin.h"

#include "resources.h"
#include "Util.h"
#include "Web.h"

#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "d3d11.h"
#include "detours.h"
#include "imgui.h"
#include "nlohmann/json.hpp"

using namespace PSO2TestPlugin;

typedef HRESULT(WINAPI *CreateDeviceAndSwapChain)(IDXGIAdapter               *pAdapter,
                                                  D3D_DRIVER_TYPE            DriverType,
                                                  HMODULE                    Software,
                                                  UINT                       Flags,
                                                  const D3D_FEATURE_LEVEL    *pFeatureLevels,
                                                  UINT                       FeatureLevels,
                                                  UINT                       SDKVersion,
                                                  const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                  IDXGISwapChain             **ppSwapChain,
                                                  ID3D11Device               **ppDevice,
                                                  D3D_FEATURE_LEVEL          *pFeatureLevel,
                                                  ID3D11DeviceContext        **ppImmediateContext);

typedef HRESULT(__fastcall *Present)(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags);

struct D3D11VTable {
    Present Present;
};

static WNDPROC gameWindowProc = nullptr;
static CreateDeviceAndSwapChain oCreateDeviceAndSwapChain;
static IDXGISwapChain *stolenSwapChain;
static ID3D11Device *device;
static ID3D11DeviceContext *context;
static ID3D11RenderTargetView *mainRenderTargetView;
static D3D11VTable *d3d11VTable;
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

D3D11VTable* BuildD3D11VTable(IDXGISwapChain *swapChainFrom) {
    auto dxVTable = reinterpret_cast<DWORD_PTR*>(swapChainFrom);
    dxVTable = reinterpret_cast<DWORD_PTR*>(dxVTable[0]);
    auto oPresent = reinterpret_cast<Present>(dxVTable[8]);

    auto dxVTableCopy = new D3D11VTable{oPresent};

    return dxVTableCopy;
}

void InitImGui() {
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
    ImGui_ImplDX11_Init(device, context);
    ImGui::GetIO().ImeWindowHandle = gameHWnd;

    ID3D11Texture2D *backBuffer;
    stolenSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBuffer));
    device->CreateRenderTargetView(backBuffer, nullptr, &mainRenderTargetView);
    backBuffer->Release();
}

HRESULT WINAPI HookedPresent(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) {
    if (gameWindowProc == nullptr) {
        InitImGui();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = ImGui::GetIO().WantCaptureMouse;

    if (show) {
        DrawManager->Execute();
    }

    ImGui::Render();
    context->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return d3d11VTable->Present(swapChain, syncInterval, flags);
}

HRESULT HookedD3D11CreateDeviceAndSwapChain(IDXGIAdapter               *pAdapter,
                                            D3D_DRIVER_TYPE            DriverType,
                                            HMODULE                    Software,
                                            UINT                       Flags,
                                            const D3D_FEATURE_LEVEL    *pFeatureLevels,
                                            UINT                       FeatureLevels,
                                            UINT                       SDKVersion,
                                            const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                            IDXGISwapChain             **ppSwapChain,
                                            ID3D11Device               **ppDevice,
                                            D3D_FEATURE_LEVEL          *pFeatureLevel,
                                            ID3D11DeviceContext        **ppImmediateContext) {
    auto res = oCreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
    if (SUCCEEDED(res)) {
        stolenSwapChain = *ppSwapChain;
        device = *ppDevice;
        context = *ppImmediateContext;

        d3d11VTable = BuildD3D11VTable(stolenSwapChain);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&reinterpret_cast<PVOID&>(d3d11VTable->Present), reinterpret_cast<PVOID>(HookedPresent));
        DetourTransactionCommit();
    }
    return res;
}

BOOL WINAPI PSO2TestPlugin::Hook() {
    DrawManager = new Interface::InterfaceManager();

    Initialize();

    oCreateDeviceAndSwapChain = &D3D11CreateDeviceAndSwapChain;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&reinterpret_cast<PVOID&>(oCreateDeviceAndSwapChain), reinterpret_cast<PVOID>(HookedD3D11CreateDeviceAndSwapChain));
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