#include <windows.h>
#include <cstdio>
#include "init.h"
#include "hooks.h"
#include "hooks_game.h"

static void Log(const char* msg) {
    FILE* f = fopen("camus_debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

DWORD WINAPI CheatThread(LPVOID lpParam) {
    Log("step 1: AllocConsole...");
    AllocConsole();
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    Log("step 2: console done");

    Sleep(100);

    Log("step 3: InitEverything...");
    InitEverything();
    Log("step 4: InitEverything done");

    Log("step 5: Hooks::initialize (D3D11)...");
    if (!Hooks::initialize()) {
        Log("step 5: D3D11 FAILED");
        return 1;
    }
    Log("step 6: cheat running");

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(1);
    }

    Hooks::ShutdownGameHooks();
    Hooks::shutdown();
    FreeConsole();
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
