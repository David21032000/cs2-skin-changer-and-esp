#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "hooks.h"
#include "hooks_game.h"
#include "menu.h"
#include "init.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffers_t)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

namespace {
    Present_t originalPresent = nullptr;
    ResizeBuffers_t originalResizeBuffers = nullptr;
    void** g_vtableHooked = nullptr;

    ID3D11Device* g_pDevice = nullptr;
    ID3D11DeviceContext* g_pContext = nullptr;
    HWND g_hWindow = nullptr;
    WNDPROC g_originalWndProc = nullptr;
    bool g_imguiInit = false;
    bool g_cheatInit = false;
}

static void Log(const char* msg) {
    FILE* f = fopen("camus_debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_imguiInit && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        g_MenuOpen = !g_MenuOpen;
        return true;
    }
    return CallWindowProcA(g_originalWndProc, hWnd, msg, wParam, lParam);
}

static void InitCheat() {
    Log("ch init: console...");
    AllocConsole();
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    printf("CAMUS CS2 - init started\n");

    Log("ch init: InitEverything...");
    InitEverything();
    Log("ch init: InitEverything done");

    Log("ch init: InitGameHooks...");
    Hooks::InitGameHooks();
    Log("ch init: InitGameHooks done");
}

HRESULT __stdcall Present_hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_cheatInit) {
        g_cheatInit = true;
        Log("hook: first Present, initializing cheat...");
        InitCheat();

        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice))) {
            g_pDevice->GetImmediateContext(&g_pContext);
            DXGI_SWAP_CHAIN_DESC desc;
            if (SUCCEEDED(pSwapChain->GetDesc(&desc))) {
                g_hWindow = desc.OutputWindow;
                g_originalWndProc = (WNDPROC)SetWindowLongPtrA(g_hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

                if (ImGui_ImplWin32_Init(g_hWindow) && ImGui_ImplDX11_Init(g_pDevice, g_pContext)) {
                    g_imguiInit = true;
                    Log("hook: ImGui init done");
                } else {
                    Log("hook: ImGui init FAILED");
                }
            }
        }
        Log("hook: full init done");
    }

    if (!g_imguiInit) {
        return originalPresent(pSwapChain, SyncInterval, Flags);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Hooks::GameLoop();
    Menu::Render();
    Menu::RenderWatermark();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return originalPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall ResizeBuffers_hook(IDXGISwapChain* pSwapChain, UINT BufferCount,
    UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    if (g_imguiInit) {
        ImGui_ImplDX11_InvalidateDeviceObjects();
    }
    HRESULT hr = originalResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (g_imguiInit && SUCCEEDED(hr)) {
        ImGui_ImplDX11_CreateDeviceObjects();
    }
    return hr;
}

bool Hooks::initialize() {
    Log("init: creating temp D3D11 device for vtable hook");
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = 1;
    scd.BufferDesc.Height = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = GetDesktopWindow();
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    IDXGISwapChain* tempSwapChain = nullptr;
    ID3D11Device* tempDevice = nullptr;
    ID3D11DeviceContext* tempContext = nullptr;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &scd,
        &tempSwapChain, &tempDevice, nullptr, &tempContext
    );
    if (FAILED(hr) || !tempSwapChain || !tempDevice) {
        Log("init: D3D11 device creation FAILED");
        return false;
    }
    Log("init: D3D11 OK");

    void** vtable = *(void***)tempSwapChain;
    g_vtableHooked = vtable;

    DWORD oldProtect;
    VirtualProtect(&vtable[8], sizeof(void*), PAGE_READWRITE, &oldProtect);
    originalPresent = (Present_t)vtable[8];
    vtable[8] = (void*)&Present_hook;
    VirtualProtect(&vtable[8], sizeof(void*), oldProtect, &oldProtect);

    VirtualProtect(&vtable[13], sizeof(void*), PAGE_READWRITE, &oldProtect);
    originalResizeBuffers = (ResizeBuffers_t)vtable[13];
    vtable[13] = (void*)&ResizeBuffers_hook;
    VirtualProtect(&vtable[13], sizeof(void*), oldProtect, &oldProtect);

    Log("init: vtable hooked via temp swapchain");

    tempSwapChain->Release();
    tempDevice->Release();
    tempContext->Release();
    Log("init: done");
    return true;
}

void Hooks::shutdown() {
    if (g_vtableHooked) {
        DWORD oldProtect;
        if (originalPresent) {
            VirtualProtect(&g_vtableHooked[8], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_vtableHooked[8] = (void*)originalPresent;
            VirtualProtect(&g_vtableHooked[8], sizeof(void*), oldProtect, &oldProtect);
        }
        if (originalResizeBuffers) {
            VirtualProtect(&g_vtableHooked[13], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_vtableHooked[13] = (void*)originalResizeBuffers;
            VirtualProtect(&g_vtableHooked[13], sizeof(void*), oldProtect, &oldProtect);
        }
        g_vtableHooked = nullptr;
    }

    if (g_imguiInit) {
        if (g_originalWndProc && g_hWindow)
            SetWindowLongPtrA(g_hWindow, GWLP_WNDPROC, (LONG_PTR)g_originalWndProc);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        if (g_pContext) g_pContext->Release();
        if (g_pDevice) g_pDevice->Release();
        g_imguiInit = false;
    }
}
