#include "init.h"
#include "offsets.h"
#include "interfaces.h"
#include "hooks_game.h"
#include "hooks.h"
#include <cstdio>
#include <cstdarg>
#include <Windows.h>

static void Log(const char* msg) {
    FILE* f = fopen("camus_debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

static uintptr_t GetBase(const char* mod) {
    return reinterpret_cast<uintptr_t>(GetModuleHandleA(mod));
}

void ResolveOffsets() {
    Log("init: resolving offsets...");

    Offsets::client = GetBase("client.dll");
    Offsets::engine = GetBase("engine2.dll");
    Offsets::schemasystem = GetBase("schemasystem.dll");
    Offsets::materialsystem2 = GetBase("materialsystem2.dll");
    Offsets::vguimatsurface = GetBase("vguimatsurface.dll");

    char buf[256];
    snprintf(buf, sizeof(buf), "init: client.dll at 0x%llX", (unsigned long long)Offsets::client);
    Log(buf);
    snprintf(buf, sizeof(buf), "init: engine2.dll at 0x%llX", (unsigned long long)Offsets::engine);
    Log(buf);

    if (!Offsets::client) { Log("init: FAILED - client.dll not found"); return; }

    // These are RVAs from a2x/cs2-dumper (2026-07-21)
    Offsets::dwLocalPlayerPawn = Offsets::client + 0x23A5CB8;
    Offsets::dwEntityList = Offsets::client + 0x254CFF0;
    Offsets::dwViewMatrix = Offsets::client + 0x23A89C0;
    Offsets::dwGlobalVars = Offsets::client + 0x2091660;
    Offsets::dwViewAngles = Offsets::client + 0x23BB3D8;
    Offsets::dwSensitivity = Offsets::client + 0x23A3B68;
    Offsets::dwGameRules = Offsets::client + 0x23A6A18;
    Offsets::dwInputSystem = Offsets::client + 0x23BB150; // dwCSGOInput

    {
        uintptr_t addr = Mem::FindPattern("client.dll",
            "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x0F\xB6\x40",
            "xxx????xxxx?xxx");
        if (addr) Offsets::dwForceJump = Mem::ResolveRelativeAddress(addr);
        else Log("init: dwForceJump pattern FAILED");
    }
    {
        uintptr_t addr = Mem::FindPattern("client.dll",
            "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x0F\xB6\x40\x3C",
            "xxx????xxxx?xxxx");
        if (addr) Offsets::dwForceAttack = Mem::ResolveRelativeAddress(addr);
        else Log("init: dwForceAttack pattern FAILED");
    }

    Log("init: offsets resolved OK");
}

bool InitEverything() {
    Log("init: starting...");
    ResolveOffsets();
    Log("init: grabbing interfaces...");
    Interfaces::InitializeAll();
    Log("init: interfaces OK");
    Log("init: initializing game hooks...");
    if (!Hooks::InitGameHooks()) {
        Log("init: game hooks FAILED (features won't work, menu/visuals OK)");
    }
    Log("init: done");
    return true;
}
