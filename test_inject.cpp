#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        MessageBoxA(NULL, "Injected successfully!", "Test", MB_OK);
    }
    return TRUE;
}
