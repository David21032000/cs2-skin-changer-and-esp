#include "process.h"
#include <cstring>

namespace Process {

DWORD FindProcessId(const char* processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snapshot, &pe)) {
        do {
            char name[260];
            WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, name, sizeof(name), nullptr, nullptr);

            if (_stricmp(name, processName) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return pid;
}

uintptr_t GetModuleBase(DWORD processId, const char* moduleName) {
    uintptr_t base = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    if (Module32FirstW(snapshot, &me)) {
        do {
            char name[260];
            WideCharToMultiByte(CP_UTF8, 0, me.szModule, -1, name, sizeof(name), nullptr, nullptr);

            if (_stricmp(name, moduleName) == 0) {
                base = reinterpret_cast<uintptr_t>(me.modBaseAddr);
                break;
            }
        } while (Module32NextW(snapshot, &me));
    }

    CloseHandle(snapshot);
    return base;
}

int GetModules(DWORD processId, ModuleInfo* outBuffer, int maxCount) {
    if (!outBuffer || maxCount <= 0) return 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    int count = 0;
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    if (Module32FirstW(snapshot, &me)) {
        do {
            if (count >= maxCount) break;

            ModuleInfo& info = outBuffer[count];
            info.baseAddress = reinterpret_cast<uintptr_t>(me.modBaseAddr);
            info.size = me.modBaseSize;
            WideCharToMultiByte(CP_UTF8, 0, me.szModule, -1, info.name, sizeof(info.name), nullptr, nullptr);
            WideCharToMultiByte(CP_UTF8, 0, me.szExePath, -1, info.path, sizeof(info.path), nullptr, nullptr);

            count++;
        } while (Module32NextW(snapshot, &me));
    }

    CloseHandle(snapshot);
    return count;
}

} // namespace Process
