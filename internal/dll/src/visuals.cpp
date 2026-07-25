#include "visuals.h"
#include "menu.h"
#include "interfaces.h"
#include "math.h"
#include "netvars.h"
#include "imgui/imgui.h"
#include <cstdio>
#include <string>

extern VisualsConfig g_VisualsConfig;

struct player_info_t {
    char name[128];
    int userId;
    int64_t steamId;
};

void Visuals::Render() {
    auto draw = ImGui::GetBackgroundDrawList();
    if (!draw) return;
    auto& io = ImGui::GetIO();
    int w = io.DisplaySize.x, h = io.DisplaySize.y;
    auto entityList = Interfaces::entityList;
    if (!entityList) return;
    auto engine = Interfaces::engine;
    if (!engine || !engine->IsInGame()) return;

    uintptr_t local = Mem::Read<uintptr_t>(Offsets::dwLocalPlayerPawn);
    if (!local) return;
    int localTeam = Mem::Read<int>(local + Offsets::NetVar::m_iTeamNum);

    for (int i = 1; i <= 64; i++) {
        uintptr_t list = Mem::Read<uintptr_t>(Offsets::dwEntityList);
        if (!list) continue;
        uintptr_t entry = Mem::Read<uintptr_t>(list + i * 0x10);
        if (!entry) continue;
        uintptr_t player = Mem::Read<uintptr_t>(entry + 0x10 * (i & 0x1FF));

        if (!player) continue;
        if (Mem::Read<int>(player + Offsets::NetVar::m_lifeState)) continue;
        int hp = Mem::Read<int>(player + Offsets::NetVar::m_iHealth);
        if (hp < 1 || hp > 100) continue;
        int team = Mem::Read<int>(player + Offsets::NetVar::m_iTeamNum);
        if (team == localTeam) continue;

        Vec3 pos = Mem::Read<Vec3>(player + Offsets::NetVar::m_vecAbsOrigin);
        Vec3 head = pos; head.z += 72.f;
        Vec3 sp, sh;
        ViewMatrix vm = Mem::Read<ViewMatrix>(Offsets::dwViewMatrix);
        if (!WorldToScreen(pos, sp, vm, w, h)) continue;
        if (!WorldToScreen(head, sh, vm, w, h)) continue;

        float boxH = sp.y - sh.y;
        float boxW = boxH * 0.6f;
        if (boxH < 1) continue;

        ImColor col = (team == localTeam) ? ImColor(0,255,0) : ImColor(255,0,0);

        if (g_VisualsConfig.box)
            draw->AddRect(ImVec2(sp.x-boxW,sh.y), ImVec2(sp.x+boxW,sp.y), col);

        if (g_VisualsConfig.healthBar) {
            float hRatio = hp / 100.f;
            ImColor hc = ImColor(1.f-hRatio, hRatio, 0.f);
            draw->AddRectFilled(ImVec2(sp.x-boxW-6,sh.y), ImVec2(sp.x-boxW-3,sp.y), ImColor(0,0,0,200));
            float bh = boxH * hRatio;
            draw->AddRectFilled(ImVec2(sp.x-boxW-6,sp.y-bh), ImVec2(sp.x-boxW-3,sp.y), hc);
        }

        if (g_VisualsConfig.name) {
            player_info_t info;
            if (engine->GetPlayerInfo(i, &info))
                draw->AddText(ImVec2(sp.x-boxW,sh.y-14), col, info.name);
        }

        if (g_VisualsConfig.weapon) {
            uintptr_t weapon = 0;
            uintptr_t weaponServices = Mem::Read<uintptr_t>(player + Offsets::NetVar::m_pWeaponServices);
            if (weaponServices) {
                uintptr_t weaponHandle = Mem::Read<uintptr_t>(weaponServices + 0xC0);
                if (weaponHandle)
                    weapon = reinterpret_cast<uintptr_t>(Interfaces::entityList->GetClientEntityFromHandle(weaponHandle));
            }
            if (weapon) {
                int idx = Mem::Read<int>(weapon + Offsets::NetVar::m_iItemDefinitionIndex);
                const char* wpnNames[] = {"", "Deagle", "Dualies", "Five-7", "Glock", "AK-47", "AUG", "AWP", "FAMAS", "G3SG1", "Galil", "M4A4", "M4A1-S", "SCAR-20", "SG 553", "SSG 08", "MP9", "MP7", "MP5-SD", "P90", "UMP-45", "MAC-10", "PP-Bizon", "MAG-7", "Nova", "XM1014", "M249", "Negev", "Flash", "HE", "Smoke", "Molotov", "Decoy", "Incendiary", "C4"};
                if (idx >= 1 && idx <= 34)
                    draw->AddText(ImVec2(sp.x-boxW, sp.y+2), col, wpnNames[idx]);
            }
        }

        if (g_VisualsConfig.snaplines)
            draw->AddLine(ImVec2(w/2.f,h), ImVec2(sp.x,sp.y), col);
    }
}
