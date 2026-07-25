#include <windows.h>
#include <d3d11.h>
#include <cstdio>
#include <MinHook/MinHook.h>

static void Log(const char* msg) {
    FILE* f = fopen("camus_debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

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
    Log("step 1: AllocConsole...");
    Cheat::initialize();
    Log("step 2: console done");

    Sleep(100);

    Log("step 3: MH_Init...");
    if (MH_Initialize() != MH_OK) {
        Log("step 3: MH_Init FAILED");
        Cheat::shutdown();
        return 1;
    }
    Log("step 4: MH_Init OK");

    Log("step 5: Hooks::init...");
    if (!Hooks::initialize()) {
        Log("step 5: Hooks::init FAILED");
        MH_Uninitialize();
        Cheat::shutdown();
        return 1;
    }
    Log("step 6: Hooks::init OK");

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(1);
    }

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
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
