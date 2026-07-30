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

char g_dllDir[MAX_PATH] = {0};

static void LogToFile(const char* path, const char* msg) {
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, NULL, FILE_END);
        DWORD written;
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        WriteFile(hFile, "\r\n", 2, &written, NULL);
        CloseHandle(hFile);
    }
}

static void Log(const char* msg) {
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
    if (g_dllDir[0]) {
        char path[MAX_PATH];
        snprintf(path, MAX_PATH, "%s\\camus_debug.txt", g_dllDir);
        LogToFile(path, msg);
    }
    char path2[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA(nullptr), path2, MAX_PATH);
    char* lastSlash = strrchr(path2, '\\');
    if (lastSlash) *lastSlash = '\0';
    strcat_s(path2, "\\camus_debug.txt");
    LogToFile(path2, msg);
}

HRESULT __stdcall Present_hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
HRESULT __stdcall ResizeBuffers_hook(IDXGISwapChain* pSwapChain, UINT BufferCount,
    UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_imguiInit && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        g_MenuOpen = !g_MenuOpen;
        return true;
    }
    return CallWindowProcA(g_originalWndProc, hWnd, msg, wParam, lParam);
}

static bool HookD3D11ViaTempSwapchain() {
    Log("hook: creating temp swapchain for vtable hook...");

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "camus_hook";
    RegisterClassExA(&wc);
    HWND hHiddenWnd = CreateWindowExA(0, "camus_hook", "", WS_POPUP,
        0, 0, 2, 2, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hHiddenWnd) {
        Log("hook: failed to create hidden window");
        return false;
    }
    Log("hook: hidden window created");

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = 2;
    scd.BufferDesc.Height = 2;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hHiddenWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    char buf[128];
    snprintf(buf, sizeof(buf), "hook: BufferCount=2, SwapEffect=FLIP_DISCARD, HWND=%p", hHiddenWnd);
    Log(buf);

    IDXGISwapChain* tempSwapChain = nullptr;
    ID3D11Device* tempDevice = nullptr;
    ID3D11DeviceContext* tempContext = nullptr;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &scd,
        &tempSwapChain, &tempDevice, nullptr, &tempContext
    );
    if (FAILED(hr) || !tempSwapChain || !tempDevice) {
        Log("hook: D3D11 temp device creation FAILED");
        return false;
    }
    Log("hook: D3D11 temp device OK");

    void** vtable = *reinterpret_cast<void***>(tempSwapChain);
    g_vtableHooked = vtable;

    DWORD oldProtect;
    VirtualProtect(&vtable[8], sizeof(void*), PAGE_READWRITE, &oldProtect);
    originalPresent = reinterpret_cast<Present_t>(vtable[8]);
    vtable[8] = reinterpret_cast<void*>(Present_hook);
    VirtualProtect(&vtable[8], sizeof(void*), oldProtect, &oldProtect);

    VirtualProtect(&vtable[13], sizeof(void*), PAGE_READWRITE, &oldProtect);
    originalResizeBuffers = reinterpret_cast<ResizeBuffers_t>(vtable[13]);
    vtable[13] = reinterpret_cast<void*>(ResizeBuffers_hook);
    VirtualProtect(&vtable[13], sizeof(void*), oldProtect, &oldProtect);

    tempSwapChain->Release();
    tempDevice->Release();
    tempContext->Release();
    DestroyWindow(hHiddenWnd);
    UnregisterClassA("camus_hook", wc.hInstance);
    Log("hook: vtable hooked via temp swapchain");
    return true;
}

HRESULT __stdcall Present_hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_cheatInit) {
        g_cheatInit = true;
        Log("hook: first Present, initializing cheat...");

        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice))) {
            g_pDevice->GetImmediateContext(&g_pContext);
            DXGI_SWAP_CHAIN_DESC desc;
            if (SUCCEEDED(pSwapChain->GetDesc(&desc))) {
                g_hWindow = desc.OutputWindow;
                g_originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(g_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

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

        Log("hook: InitEverything...");
        Init::InitEverything();
        Log("hook: InitEverything done");

        Hooks::InitGameHooks();
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
    Log("init: starting temp swapchain hook...");
    return HookD3D11ViaTempSwapchain();
}

void Hooks::shutdown() {
    if (g_vtableHooked) {
        DWORD oldProtect;
        if (originalPresent) {
            VirtualProtect(&g_vtableHooked[8], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_vtableHooked[8] = reinterpret_cast<void*>(originalPresent);
            VirtualProtect(&g_vtableHooked[8], sizeof(void*), oldProtect, &oldProtect);
        }
        if (originalResizeBuffers) {
            VirtualProtect(&g_vtableHooked[13], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_vtableHooked[13] = reinterpret_cast<void*>(originalResizeBuffers);
            VirtualProtect(&g_vtableHooked[13], sizeof(void*), oldProtect, &oldProtect);
        }
        g_vtableHooked = nullptr;
    }

    if (g_imguiInit) {
        if (g_originalWndProc && g_hWindow)
            SetWindowLongPtrA(g_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_originalWndProc));
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        if (g_pContext) g_pContext->Release();
        if (g_pDevice) g_pDevice->Release();
        g_imguiInit = false;
    }
}