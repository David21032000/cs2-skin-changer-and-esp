#include "visuals.h"
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
    if (!engine) return;
    int localIdx = *(int*)((uintptr_t)engine + Offsets::dwLocalPlayerPawn);
    uintptr_t lp = *(uintptr_t*)((uintptr_t)entityList + 8 * (localIdx & 0x7FFF));
    if (!lp) return;
    int localTeam = *(int*)(lp + Offsets::NetVar::m_iTeamNum);
    for (int i = 1; i <= 64; i++) {
        uintptr_t entry = *(uintptr_t*)((uintptr_t)entityList + 8 * (i & 0x7FFF) + 0x10);
        if (!entry) continue;
        uintptr_t player = *(uintptr_t*)(entry + 0x78 * (i >> 0x1C) + 0x10);
        if (!player) continue;
        if (*(int*)(player + Offsets::NetVar::m_lifeState)) continue;
        int hp = *(int*)(player + Offsets::NetVar::m_iHealth);
        if (hp < 1 || hp > 100) continue;
        int team = *(int*)(player + Offsets::NetVar::m_iTeamNum);
        if (g_VisualsConfig.enemiesOnly && team == localTeam) continue;
        Vec3 pos = *(Vec3*)(player + Offsets::NetVar::m_vecAbsOrigin);
        Vec3 head = pos; head.z += 72.f;
        Vec3 sp, sh;
        auto vm = *(ViewMatrix*)((uintptr_t)Offsets::client + Offsets::dwViewMatrix);
        if (!WorldToScreen(pos, sp, vm, w, h)) continue;
        if (!WorldToScreen(head, sh, vm, w, h)) continue;
        float boxH = sp.y - sh.y;
        float boxW = boxH * 0.6f;
        if (boxH < 1) continue;
        ImColor col = (team == localTeam) ? ImColor(0,255,0) : ImColor(255,0,0);
        if (g_VisualsConfig.box)
            draw->AddRect(ImVec2(sp.x-boxW,sh.y), ImVec2(sp.x+boxW,sp.y), col);
        if (g_VisualsConfig.health) {
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
        if (g_VisualsConfig.snaplines)
            draw->AddLine(ImVec2(w/2,h), ImVec2(sp.x,sp.y), col);
    }
}