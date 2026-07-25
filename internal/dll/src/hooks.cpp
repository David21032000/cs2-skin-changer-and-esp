#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <MinHook/MinHook.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "hooks.h"
#include "offsets.h"
#include "menu.h"
#include <cstdio>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffers_t)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
typedef void(__stdcall* CreateMove_t)(int, float, bool);

namespace {
    Present_t originalPresent = nullptr;
    ResizeBuffers_t originalResizeBuffers = nullptr;
    CreateMove_t originalCreateMove = nullptr;

    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11Device* g_pDevice = nullptr;
    ID3D11DeviceContext* g_pContext = nullptr;
    HWND g_hWindow = nullptr;
    WNDPROC g_originalWndProc = nullptr;
    bool g_initialized = false;
}
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProcA(g_originalWndProc, hWnd, msg, wParam, lParam);
}

HRESULT __stdcall Present_hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_initialized) {
        g_pSwapChain = pSwapChain;
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice))) {
            g_pDevice->GetImmediateContext(&g_pContext);
            DXGI_SWAP_CHAIN_DESC desc;
            pSwapChain->GetDesc(&desc);
            g_hWindow = desc.OutputWindow;
            g_originalWndProc = (WNDPROC)SetWindowLongPtrA(g_hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

            ImGui_ImplWin32_Init(g_hWindow);
            ImGui_ImplDX11_Init(g_pDevice, g_pContext);
            g_initialized = true;
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Menu::Render();
    Menu::RenderWatermark();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return originalPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall ResizeBuffers_hook(IDXGISwapChain* pSwapChain, UINT BufferCount,
    UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    if (g_initialized) {
        ImGui_ImplDX11_InvalidateDeviceObjects();
        ImGui_ImplDX11_CreateDeviceObjects();
    }
    return originalResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}
void __stdcall CreateMove_hook(int sequence_number, float input_sample_frametime, bool active) {
    originalCreateMove(sequence_number, input_sample_frametime, active);

    void* cmd = nullptr;
    uintptr_t* pCmd = nullptr;

    if (!cmd) return;

    {
    }
}

void* FindPattern(const char* moduleName, const char* pattern, const char* mask) {
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (!hModule) return nullptr;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    DWORD size = ntHeaders->OptionalHeader.SizeOfImage;

    BYTE* base = (BYTE*)hModule;
    DWORD patternLen = strlen(mask);

    for (DWORD i = 0; i < size - patternLen; i++) {
        bool found = true;
        for (DWORD j = 0; j < patternLen; j++) {
            if (mask[j] == 'x' && base[i + j] != (BYTE)pattern[j]) {
                found = false;
                break;
            }
        }
        if (found)
            return &base[i];
    }
    return nullptr;
}

bool Hooks::initialize() {
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

    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &scd,
        &tempSwapChain, &tempDevice, nullptr, &tempContext
    );

    void** vtable = *(void***)tempSwapChain;

    void* presentTarget = vtable[8];
    void* resizeTarget = vtable[13];

    tempSwapChain->Release();
    tempDevice->Release();
    tempContext->Release();

    if (MH_CreateHook(presentTarget, reinterpret_cast<LPVOID>(&Present_hook), (void**)&originalPresent) != MH_OK)
        return false;

    if (MH_CreateHook(resizeTarget, reinterpret_cast<LPVOID>(&ResizeBuffers_hook), (void**)&originalResizeBuffers) != MH_OK)
        return false;

    void* createMoveTarget = FindPattern("client.dll",
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        "xxxxxxxxxxxxxxxx");
    if (createMoveTarget) {
        if (MH_CreateHook(createMoveTarget, reinterpret_cast<LPVOID>(&CreateMove_hook), (void**)&originalCreateMove) != MH_OK)
            return false;
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
        return false;

    return true;
}

void Hooks::shutdown() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);

    if (g_initialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (g_originalWndProc && g_hWindow)
            SetWindowLongPtrA(g_hWindow, GWLP_WNDPROC, (LONG_PTR)g_originalWndProc);

        if (g_pContext) g_pContext->Release();
        if (g_pDevice) g_pDevice->Release();
    }
}