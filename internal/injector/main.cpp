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

bool ReadDllToMemory(const wchar_t* dllPath, std::vector<BYTE>& outBuffer) {
    HANDLE hFile = CreateFileW(dllPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wprintf(L"[-] Failed to open DLL: %ls\n", dllPath);
        return false;
    }
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return false;
    }
    outBuffer.resize(fileSize);
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, outBuffer.data(), fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        wprintf(L"[-] Failed to read DLL\n");
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    return true;
}

PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS ntHeaders) {
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        if (rva >= section[i].VirtualAddress &&
            rva < section[i].VirtualAddress + section[i].Misc.VirtualSize)
            return &section[i];
    }
    return NULL;
}

LPVOID GetPtrFromRVA(DWORD rva, PIMAGE_NT_HEADERS ntHeaders, PBYTE imageBase) {
    PIMAGE_SECTION_HEADER section = GetEnclosingSectionHeader(rva, ntHeaders);
    if (!section) return NULL;
    return imageBase + section->PointerToRawData + (rva - section->VirtualAddress);
}

bool ManualMapDll(HANDLE hProcess, const std::vector<BYTE>& dllData) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)dllData.data();
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        wprintf(L"[-] Invalid DOS signature\n");
        return false;
    }
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(dllData.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        wprintf(L"[-] Invalid NT signature\n");
        return false;
    }

    SIZE_T imageSize = ntHeaders->OptionalHeader.SizeOfImage;
    LPVOID remoteBase = VirtualAllocEx(hProcess, NULL, imageSize,
                                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteBase) {
        wprintf(L"[-] VirtualAllocEx failed (%lu)\n", GetLastError());
        return false;
    }
    wprintf(L"[+] Allocated remote memory at %p\n", remoteBase);

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        LPVOID dest = (PBYTE)remoteBase + section[i].VirtualAddress;
        LPVOID src = (PVOID)(dllData.data() + section[i].PointerToRawData);
        SIZE_T size = section[i].SizeOfRawData;
        if (!WriteProcessMemory(hProcess, dest, src, size, NULL)) {
            wprintf(L"[-] Failed to write section %d (%lu)\n", i, GetLastError());
            return false;
        }
        wprintf(L"[+] Wrote section %d at %p (%zu bytes)\n", i, dest, size);
    }

    PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)
        GetPtrFromRVA(ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress,
                      ntHeaders, (PBYTE)dllData.data());
    if (importDesc) {
        while (importDesc->Name) {
            const char* moduleName = (const char*)GetPtrFromRVA(importDesc->Name, ntHeaders, (PBYTE)dllData.data());
            HMODULE hModule = LoadLibraryA(moduleName);
            if (!hModule) {
                wprintf(L"[-] Failed to load %s\n", moduleName);
                return false;
            }
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)
                GetPtrFromRVA(importDesc->FirstThunk, ntHeaders, (PBYTE)dllData.data());
            PIMAGE_THUNK_DATA origThunk = (PIMAGE_THUNK_DATA)
                GetPtrFromRVA(importDesc->OriginalFirstThunk, ntHeaders, (PBYTE)dllData.data());
            while (thunk->u1.AddressOfData) {
                FARPROC func;
                if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal)) {
                    func = GetProcAddress(hModule, (LPCSTR)IMAGE_ORDINAL(origThunk->u1.Ordinal));
                } else {
                    PIMAGE_IMPORT_BY_NAME importName = (PIMAGE_IMPORT_BY_NAME)
                        GetPtrFromRVA(origThunk->u1.AddressOfData, ntHeaders, (PBYTE)dllData.data());
                    func = GetProcAddress(hModule, importName->Name);
                }
                if (!func) {
                    wprintf(L"[-] Failed to get proc address\n");
                    return false;
                }
                SIZE_T written = 0;
                PIMAGE_THUNK_DATA thunkPtr = (PIMAGE_THUNK_DATA)((PBYTE)remoteBase +
                    ((PBYTE)thunk - (PBYTE)GetPtrFromRVA(
                        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress,
                        ntHeaders, (PBYTE)dllData.data())));
                WriteProcessMemory(hProcess, thunkPtr,
                    &func, sizeof(LPVOID), &written);
                ++thunk;
                ++origThunk;
            }
            ++importDesc;
        }
    }

    PIMAGE_BASE_RELOCATION reloc = (PIMAGE_BASE_RELOCATION)
        GetPtrFromRVA(ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,
                      ntHeaders, (PBYTE)dllData.data());
    if (reloc && ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
        BYTE* relocRemoteBase = (BYTE*)remoteBase;
        BYTE* localBase = (BYTE*)dllData.data();
        DWORD_PTR delta = (DWORD_PTR)remoteBase - ntHeaders->OptionalHeader.ImageBase;
        while (reloc->VirtualAddress && reloc->SizeOfBlock) {
            DWORD count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            WORD* entries = (WORD*)(reloc + 1);
            for (DWORD i = 0; i < count; ++i) {
                if (entries[i] >> 12 == IMAGE_REL_BASED_DIR64) {
                    DWORD offset = reloc->VirtualAddress + (entries[i] & 0xFFF);
                    ULONG_PTR* patchAddr = (ULONG_PTR*)(localBase + offset);
                    ULONG_PTR newVal = *patchAddr + delta;
                    WriteProcessMemory(hProcess, relocRemoteBase + offset, &newVal, sizeof(ULONG_PTR), NULL);
                } else if (entries[i] >> 12 == IMAGE_REL_BASED_HIGHLOW) {
                    DWORD offset = reloc->VirtualAddress + (entries[i] & 0xFFF);
                    DWORD* patchAddr = (DWORD*)(localBase + offset);
                    DWORD newVal = *patchAddr + (DWORD)delta;
                    WriteProcessMemory(hProcess, relocRemoteBase + offset, &newVal, sizeof(DWORD), NULL);
                }
            }
            reloc = (PIMAGE_BASE_RELOCATION)((PBYTE)reloc + reloc->SizeOfBlock);
        }
    }

    DWORD entryRva = ntHeaders->OptionalHeader.AddressOfEntryPoint;
    LPVOID remoteEntry = (PBYTE)remoteBase + entryRva;
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)remoteEntry, remoteBase, 0, NULL);
    if (!hThread) {
        wprintf(L"[-] CreateRemoteThread failed (%lu)\n", GetLastError());
        return false;
    }
    wprintf(L"[+] DllMain called at %p\n", remoteEntry);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    wprintf(L"[+] DllMain finished\n");
    return true;
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

    std::vector<BYTE> dllData;
    if (!ReadDllToMemory(dllPath, dllData)) {
        wprintf(L"[-] Make sure '%ls' exists next to the injector\n", dllPath);
        CloseHandle(hProcess);
        wait_and_exit(1);
    }
    wprintf(L"[+] Read DLL (%zu bytes)\n", dllData.size());

    if (!ManualMapDll(hProcess, dllData)) {
        wprintf(L"[-] Manual mapping failed\n");
        CloseHandle(hProcess);
        wait_and_exit(1);
    }

    CloseHandle(hProcess);
    wprintf(L"[+] Injection completed successfully\n");
    wait_and_exit(0);
}
