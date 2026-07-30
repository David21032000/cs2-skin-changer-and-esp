#include <windows.h>
#include "hooks.h"

extern char g_dllDir[];

static DWORD WINAPI InitThread(LPVOID lpParam) {
    Sleep(3000);
    Hooks::initialize();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        GetModuleFileNameA(hModule, g_dllDir, MAX_PATH);
        char* lastSlash = strrchr(g_dllDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        CreateThread(nullptr, 0, InitThread, hModule, 0, nullptr);
    }
    return TRUE;
}