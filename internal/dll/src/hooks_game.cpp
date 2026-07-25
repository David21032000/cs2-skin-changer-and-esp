#include "hooks_game.h"
#include "init.h"
#include "offsets.h"
#include "interfaces.h"
#include "menu.h"
#include "aimbot.h"
#include "legitbot.h"
#include "anti_aim.h"
#include "movement.h"
#include "visuals.h"
#include "misc.h"
#include <cstdio>
#include <cstdint>
#include <cmath>

static void Log(const char* msg) {
    FILE* f = fopen("camus_debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

namespace {
    void** g_inputVtable = nullptr;
    void* g_originalCreateMove = nullptr;
    void* g_originalGetUserCmd = nullptr;
    bool g_gameHooksActive = false;

    using CreateMoveFn = void(__fastcall*)(void*, int, float, bool, bool*);
    using GetUserCmdFn = void*(__fastcall*)(void*, int);

    CInput* g_pInput = nullptr;
    bool g_thirdpersonActive = false;
}

void __fastcall CreateMove_hook(void* thisptr, int sequence_number,
    float input_sample_frametime, bool active, bool* sendPacket)
{
    if (!g_originalCreateMove || !g_originalGetUserCmd || !thisptr) return;

    CreateMoveFn original = (CreateMoveFn)g_originalCreateMove;
    original(thisptr, sequence_number, input_sample_frametime, active, sendPacket);

    if (!active) return;

    GetUserCmdFn getCmd = (GetUserCmdFn)g_originalGetUserCmd;
    CUserCmd* cmd = (CUserCmd*)getCmd(thisptr, sequence_number);
    if (!cmd) return;

    Aimbot::Run(cmd);
    Legitbot::Run(cmd);
    Movement::Run(cmd);
    AntiAim::Run(cmd, sendPacket);

    // ── Double Tap ──────────────────────────────────────────
    static bool dtCharged = false;
    if (g_RageConfig.doubleTap && sendPacket) {
        bool attacking = (cmd->buttons & (1 << 0));
        bool released = false;
        if (dtCharged) {
            cmd->buttons |= (1 << 0);
            *sendPacket = true;
            dtCharged = false;
            released = true;
        }
        if (!released && attacking) {
            dtCharged = true;
            *sendPacket = false;
        }
    } else {
        dtCharged = false;
    }
}

bool Hooks::InitGameHooks() {
    Log("g_hooks: init...");

    if (!Offsets::client) {
        Log("g_hooks: client.dll not loaded");
        return false;
    }

    uintptr_t inputPtrAddr = Offsets::dwInputSystem;
    g_pInput = *(CInput**)inputPtrAddr;

    if (!g_pInput || !g_pInput->vtable) {
        char buf[256];
        snprintf(buf, sizeof(buf), "g_hooks: CInput not found at 0x%llX", (unsigned long long)inputPtrAddr);
        Log(buf);
        snprintf(buf, sizeof(buf), "g_hooks: value: 0x%llX", (unsigned long long)(*(uintptr_t*)inputPtrAddr));
        Log(buf);
        return false;
    }
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "g_hooks: CInput at 0x%llX, vtable at 0x%llX",
            (unsigned long long)(uintptr_t)g_pInput,
            (unsigned long long)(uintptr_t)g_pInput->vtable);
        Log(buf);
    }

    g_inputVtable = g_pInput->vtable;

    DWORD oldProtect;
    VirtualProtect(&g_inputVtable[1], sizeof(void*), PAGE_READWRITE, &oldProtect);
    g_originalCreateMove = g_inputVtable[1];
    g_inputVtable[1] = (void*)&CreateMove_hook;
    VirtualProtect(&g_inputVtable[1], sizeof(void*), oldProtect, &oldProtect);

    g_originalGetUserCmd = g_inputVtable[2];

    g_gameHooksActive = true;
    Log("g_hooks: CreateMove hooked OK");
    return true;
}

void Hooks::ShutdownGameHooks() {
    if (g_inputVtable && g_originalCreateMove) {
        DWORD oldProtect;
        VirtualProtect(&g_inputVtable[1], sizeof(void*), PAGE_READWRITE, &oldProtect);
        g_inputVtable[1] = g_originalCreateMove;
        VirtualProtect(&g_inputVtable[1], sizeof(void*), oldProtect, &oldProtect);
    }
    g_gameHooksActive = false;
}

void Hooks::GameLoop() {
    if (!Interfaces::engine || !Interfaces::engine->IsInGame()) return;

    if (g_VisualsConfig.thirdperson != g_thirdpersonActive) {
        g_thirdpersonActive = g_VisualsConfig.thirdperson;
        if (g_thirdpersonActive) {
            Interfaces::engine->ExecuteClientCmd("thirdperson");
        } else {
            Interfaces::engine->ExecuteClientCmd("firstperson");
        }
    }

    Visuals::Render();
    Misc::SpectatorList();
}
