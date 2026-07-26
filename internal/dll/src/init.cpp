#include "init.h"
#include "interfaces.h"
#include "offsets.h"
#include "memory.h"
#include <cstdio>
#include <cstring>

static void Log(const char* msg) {
    char path[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA(nullptr), path, MAX_PATH);
    char* lastSlash = strrchr(path, '\\');
    if (lastSlash) *lastSlash = '\0';
    strcat_s(path, "\\camus_debug.txt");
    FILE* f = fopen(path, "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

static uintptr_t PatternScan(const char* moduleName, const char* pattern, const char* mask) {
    HMODULE mod = GetModuleHandleA(moduleName);
    if (!mod) return 0;

    MODULEINFO info{};
    GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(info));
    if (!info.lpBaseOfDll || !info.SizeOfImage) return 0;

    uintptr_t base = reinterpret_cast<uintptr_t>(info.lpBaseOfDll);
    size_t size = info.SizeOfImage;
    size_t patternLen = strlen(mask);

    for (size_t i = 0; i < size - patternLen; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (mask[j] == 'x' && *reinterpret_cast<uint8_t*>(base + i + j) != static_cast<uint8_t>(pattern[j])) {
                found = false;
                break;
            }
        }
        if (found) return base + i;
    }
    return 0;
}

static uintptr_t FindPatternIDA(const char* moduleName, const char* idaPattern) {
    std::string p = idaPattern;
    p.erase(std::remove(p.begin(), p.end(), ' '), p.end());

    std::string pattern;
    std::string mask;
    for (size_t i = 0; i < p.length(); i += 2) {
        std::string byteStr = p.substr(i, 2);
        if (byteStr[0] == '?' || byteStr[1] == '?') {
            pattern += '\x00';
            mask += '?';
        } else {
            pattern += static_cast<char>(std::stoi(byteStr, nullptr, 16));
            mask += 'x';
        }
    }
    return PatternScan(moduleName, pattern.c_str(), mask.c_str());
}

static uintptr_t ResolveRelative(uintptr_t addr, int offsetOffset, int instructionSize) {
    if (!addr) return 0;
    int32_t offset = Mem::Read<int32_t>(addr + offsetOffset);
    return addr + instructionSize + offset;
}

bool Init::ResolveOffsets() {
    Log("init: resolving module bases...");

    Offsets::client = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    Offsets::engine = reinterpret_cast<uintptr_t>(GetModuleHandleA("engine2.dll"));
    Offsets::schemasystem = reinterpret_cast<uintptr_t>(GetModuleHandleA("schemasystem.dll"));
    Offsets::tier0 = reinterpret_cast<uintptr_t>(GetModuleHandleA("tier0.dll"));

    if (!Offsets::client || !Offsets::engine || !Offsets::schemasystem) {
        Log("init: critical modules not loaded");
        return false;
    }
    Log("init: module bases resolved");

    struct Sig { const char* name; const char* module; const char* pattern; };
    
    Sig signatures[] = {
        {"dwLocalPlayerPawn", "client.dll", "48 8B 0D ? ? ? ? 48 85 C9 74 ? 8B 01 FF 50 ? 48 8B 0D"},
        {"dwEntityList", "client.dll", "48 8B 0D ? ? ? ? 48 8B 01 44 8D 43"},
        {"dwViewMatrix", "client.dll", "48 8D 0D ? ? ? ? 48 C1 E0 06"},
        {"dwInputSystem", "client.dll", "48 89 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01"},
        {"dwForceJump", "client.dll", "48 89 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01 8B"},
        {"dwForceAttack", "client.dll", "48 89 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01 8B"},
        {"dwGlobalVars", "client.dll", "48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 05"},
        {"dwSensitivity", "client.dll", "F3 0F 10 05 ? ? ? ? F3 0F 11 44 24"},
        {"dwMouseEnable", "client.dll", "80 3D ? ? ? ? ? 75 ? 48 8B 0D"},
        {nullptr, nullptr, nullptr}
    };

    for (int i = 0; signatures[i].name; ++i) {
        uintptr_t addr = FindPatternIDA(signatures[i].module, signatures[i].pattern);
        if (!addr) {
            char buf[256];
            snprintf(buf, sizeof(buf), "init: PATTERN FAILED: %s", signatures[i].name);
            Log(buf);
            return false;
        }

        uintptr_t resolved = 0;
        if (strstr(signatures[i].pattern, "? ? ? ?") == signatures[i].pattern + 3) {
            resolved = ResolveRelative(addr, 3, 7);
        } else {
            resolved = addr;
        }

        if (!resolved) {
            char buf[256];
            snprintf(buf, sizeof(buf), "init: RELATIVE FAILED: %s", signatures[i].name);
            Log(buf);
            return false;
        }

        if (strcmp(signatures[i].name, "dwLocalPlayerPawn") == 0) Offsets::dwLocalPlayerPawn = resolved;
        else if (strcmp(signatures[i].name, "dwEntityList") == 0) Offsets::dwEntityList = resolved;
        else if (strcmp(signatures[i].name, "dwViewMatrix") == 0) Offsets::dwViewMatrix = resolved;
        else if (strcmp(signatures[i].name, "dwInputSystem") == 0) Offsets::dwInputSystem = resolved;
        else if (strcmp(signatures[i].name, "dwForceJump") == 0) Offsets::dwForceJump = resolved;
        else if (strcmp(signatures[i].name, "dwForceAttack") == 0) Offsets::dwForceAttack = resolved;
        else if (strcmp(signatures[i].name, "dwGlobalVars") == 0) Offsets::dwGlobalVars = resolved;
        else if (strcmp(signatures[i].name, "dwSensitivity") == 0) Offsets::dwSensitivity = resolved;
        else if (strcmp(signatures[i].name, "dwMouseEnable") == 0) Offsets::dwMouseEnable = resolved;

        char buf[256];
        snprintf(buf, sizeof(buf), "init: %s = 0x%llX", signatures[i].name, (unsigned long long)resolved);
        Log(buf);
    }

    Log("init: resolving netvars via schema system...");
    Interfaces::InitializeAll();

    if (!Offsets::NetVar::m_iHealth || !Offsets::NetVar::m_iTeamNum || 
        !Offsets::NetVar::m_vecAbsOrigin || !Offsets::NetVar::m_angEyeAngles) {
        Log("init: critical netvars missing");
        return false;
    }

    Log("init: all offsets resolved successfully");
    return true;
}

void Init::InitEverything() {
    Log("InitEverything: started");
    if (!ResolveOffsets()) {
        Log("InitEverything: ResolveOffsets FAILED");
        return;
    }
    Log("InitEverything: all offsets ready");
}