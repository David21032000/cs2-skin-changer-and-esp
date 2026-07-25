#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <shellapi.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "psapi.lib")

DWORD FindProcessIdA(const char* processName) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
    if (Process32First(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static void wait_and_exit(int code) {
    printf("\nPress any key to exit...");
    getchar();
    exit(code);
}

static void launch_cs2() {
    printf("[!] CS2 not running. Launching CS2 via Steam...\n");
    ShellExecuteA(NULL, "open", "steam://rungameid/730", NULL, NULL, SW_SHOWNORMAL);
}

int main(int argc, char* argv[]) {
    const char* targetProcess = "cs2.exe";

    DWORD pid = FindProcessIdA(targetProcess);
    if (!pid) {
        launch_cs2();
        printf("[!] Waiting for game to launch...\n");
        while (!pid) {
            Sleep(2000);
            pid = FindProcessIdA(targetProcess);
            printf(".");
        }
        printf("\n");
    }
    printf("[+] Found %s (PID: %lu)\n", targetProcess, pid);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        wprintf(L"[-] OpenProcess failed (%lu). Try running as Administrator!\n", GetLastError());
        wait_and_exit(1);
    }
    wprintf(L"[+] Opened handle to process\n");

    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(NULL, dllPath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(dllPath, L'\\');
    if (lastSlash) *lastSlash = L'\0';
    wcsncat_s(dllPath, MAX_PATH, L"\\camus.dll", _TRUNCATE);
    wprintf(L"[+] DLL path: %ls\n", dllPath);

    // Allocate memory in the target process for the DLL path string
    size_t pathLen = (wcslen(dllPath) + 1) * sizeof(wchar_t);
    LPVOID remotePath = VirtualAllocEx(hProcess, NULL, pathLen, MEM_COMMIT, PAGE_READWRITE);
    if (!remotePath) {
        wprintf(L"[-] VirtualAllocEx failed (%lu)\n", GetLastError());
        CloseHandle(hProcess);
        wait_and_exit(1);
    }

    if (!WriteProcessMemory(hProcess, remotePath, dllPath, pathLen, NULL)) {
        wprintf(L"[-] WriteProcessMemory failed (%lu)\n", GetLastError());
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wait_and_exit(1);
    }

    // Get the address of LoadLibraryW in the target process
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryW");
    if (!loadLibraryAddr) {
        wprintf(L"[-] GetProcAddress(LoadLibraryW) failed\n");
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wait_and_exit(1);
    }

    // Create remote thread to call LoadLibraryW(dllPath)
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);
    if (!hThread) {
        wprintf(L"[-] CreateRemoteThread failed (%lu)\n", GetLastError());
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wait_and_exit(1);
    }

    wprintf(L"[+] Waiting for LoadLibrary...\n");
    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    CloseHandle(hThread);

    if (exitCode == 0) {
        wprintf(L"[-] LoadLibrary failed (DLL not found or dependency issue)\n");
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wait_and_exit(1);
    }

    wprintf(L"[+] DLL loaded at 0x%08lX\n", exitCode);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);

    wprintf(L"[+] Injection completed successfully\n");
    wprintf(L"[+] Waiting for CS2 to exit (close the game to see debug log)...\n");

    WaitForSingleObject(hProcess, INFINITE);
    GetExitCodeProcess(hProcess, &exitCode);

    wprintf(L"\n[!] CS2 exited with code 0x%08lX\n", exitCode);

    printf("\n\n=== camus_debug.txt ===\n");
    FILE* f = fopen("camus_debug.txt", "r");
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) printf("%s", buf);
        fclose(f);
    } else {
        printf("(no debug file found)\n");
    }
    printf("========================\n");

    CloseHandle(hProcess);
    wait_and_exit(exitCode == 0 ? 0 : 1);
}
