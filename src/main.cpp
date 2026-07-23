#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "offsets.h"
#include "process.h"
#include "skin_changer.h"
#include "esp.h"
#include "ui.h"
#include <cstdio>

static SkinChanger g_changer;
static Esp g_esp;
static bool g_menuOpen = false;
static bool g_connected = false;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static HWND g_hwnd = nullptr;

static constexpr float ChromaR = 0.0f;
static constexpr float ChromaG = 1.0f;
static constexpr float ChromaB = 0.0f;
static constexpr DWORD ChromaKey = RGB(0, 255, 0);

bool CreateDeviceD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevelArray[] = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext
    );

    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
            createFlags, featureLevelArray, 2,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain,
            &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext
        );
    }

    if (FAILED(hr)) return false;

    ID3D11Texture2D* backBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) return false;
    hr = g_pd3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_mainRenderTargetView);
    backBuffer->Release();
    return SUCCEEDED(hr);
}

void CleanupDeviceD3D() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (g_pd3dDevice && wParam != SIZE_MINIMIZED) {
                if (g_mainRenderTargetView) {
                    g_mainRenderTargetView->Release();
                    g_mainRenderTargetView = nullptr;
                }
                g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                ID3D11Texture2D* backBuffer = nullptr;
                g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
                g_pd3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_mainRenderTargetView);
                backBuffer->Release();
            }
            return 0;
        case WM_HOTKEY:
            if (wParam == 1) {
                g_menuOpen = !g_menuOpen;
                LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                if (g_menuOpen) {
                    exStyle &= ~WS_EX_TRANSPARENT;
                } else {
                    exStyle |= WS_EX_TRANSPARENT;
                }
                SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
            }
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void PositionOverCS2() {
    HWND cs2Hwnd = FindWindowW(nullptr, L"Counter-Strike 2");
    if (!cs2Hwnd) cs2Hwnd = FindWindowW(nullptr, L"Counter-Strike");
    if (!cs2Hwnd || !g_hwnd) return;

    RECT rect;
    GetWindowRect(cs2Hwnd, &rect);
    SetWindowPos(g_hwnd, HWND_TOPMOST,
        rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top,
        SWP_NOACTIVATE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"CS2SkinChangerClass";

    if (!RegisterClassExW(&wc)) return 1;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        wc.lpszClassName, L"CS2 Skin Changer",
        WS_POPUP,
        0, 0, screenW, screenH,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_hwnd) return 1;

    SetLayeredWindowAttributes(g_hwnd, ChromaKey, 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        DestroyWindow(g_hwnd);
        UnregisterClassW(wc.lpszClassName, hInstance);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOW);
    RegisterHotKey(g_hwnd, 1, 0, VK_INSERT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool running = true;
    MSG msg = {};

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        PositionOverCS2();

        if (!g_connected) {
            if (g_changer.Initialize(Offsets::ProcessName)) {
                g_changer.LoadConfig("skins.json");
                g_connected = true;
            }
        }

        if (g_connected && !g_changer.IsConnected()) {
            g_connected = false;
        }

        if (g_connected) {
            g_changer.Tick();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (g_menuOpen) {
            RenderUI(g_changer, g_connected, g_esp.config);
        }

        if (g_connected && g_esp.config.enabled) {
            g_esp.Render(g_changer.GetMemory(), g_changer.GetClientBase());
        }

        ImGui::Render();
        const float clearColor[4] = { ChromaR, ChromaG, ChromaB, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);

        Sleep(5);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(wc.lpszClassName, hInstance);

    return 0;
}
