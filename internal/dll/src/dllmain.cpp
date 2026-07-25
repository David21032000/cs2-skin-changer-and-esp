#include <windows.h>
#include <cstdio>
#include "init.h"
#include "hooks.h"
#include "hooks_game.h"

static void Log(const char* msg) {
    FILE* f = fopen("camus_debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        Log("dll: attached, initializing D3D11 hook...");
        Hooks::initialize();
        Log("dll: D3D11 hook done, waiting for Present...");
    }
    return TRUE;
}
