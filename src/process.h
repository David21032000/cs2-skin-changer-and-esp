#pragma once
#include <cstdint>
#include <windows.h>
#include <tlhelp32.h>

namespace Process {

struct ModuleInfo {
    uintptr_t baseAddress = 0;
    size_t size = 0;
    char name[260]{};
    char path[MAX_PATH]{};
};

DWORD FindProcessId(const char* processName);
uintptr_t GetModuleBase(DWORD processId, const char* moduleName);
int GetModules(DWORD processId, ModuleInfo* outBuffer, int maxCount);

} // namespace Process
