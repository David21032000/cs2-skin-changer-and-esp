#include <windows.h>
#include "hooks.h"

extern char g_dllDir[];

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        GetModuleFileNameA(hModule, g_dllDir, MAX_PATH);
        char* lastSlash = strrchr(g_dllDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        Hooks::initialize();
    }
    return TRUE;
}