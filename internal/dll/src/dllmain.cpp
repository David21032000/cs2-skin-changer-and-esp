#include <windows.h>
#include <d3d11.h>
#include <cstdio>
#include <MinHook/MinHook.h>

namespace Hooks {
    bool initialize();
    void shutdown();
}

namespace Cheat {
    bool initialize() {
        AllocConsole();
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        return true;
    }
    void shutdown() {
        if (GetConsoleWindow())
            FreeConsole();
    }
}

DWORD WINAPI CheatThread(LPVOID lpParam) {
    while (!GetConsoleWindow()) Sleep(100);

    if (MH_Initialize() != MH_OK) {
        Cheat::shutdown();
        return 1;
    }

    Cheat::initialize();

    if (!Hooks::initialize()) {
        Cheat::shutdown();
        return 1;
    }

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(1);
    }

    Hooks::shutdown();
    Cheat::shutdown();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, CheatThread, hModule, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}
