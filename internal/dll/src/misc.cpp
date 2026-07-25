#include "misc.h"
#include "interfaces.h"
#include "math.h"
#include "offsets.h"
#include "aimbot.h"
#include "imgui/imgui.h"
#include <cstdio>
#include <string>
#include <vector>

extern MiscConfig g_MiscConfig;

void Misc::Run(void* cmdPtr) {
    auto cmd = (CUserCmd*)cmdPtr;
    if (!cmd) return;
}

void Misc::RenderWatermark() {
    if (!g_MiscConfig.watermark) return;
    auto draw = ImGui::GetForegroundDrawList();
    if (!draw) return;
    char buf[128];
    snprintf(buf, 128, "Camus CS2 | FPS: %.0f", ImGui::GetIO().Framerate);
    draw->AddText(ImVec2(10,10), ImColor(0,191,255), buf);
}

void Misc::SpectatorList() {
    if (!g_MiscConfig.spectatorList) return;
    auto draw = ImGui::GetForegroundDrawList();
    if (!draw) return;
    auto el = Interfaces::entityList;
    auto eng = Interfaces::engine;
    if (!el || !eng) return;
    uintptr_t local = Mem::Read<uintptr_t>(Offsets::dwLocalPlayerPawn);
    if (!local) return;
    std::vector<std::string> sp;
    for (int i = 1; i <= 64; i++) {
        uintptr_t e = *(uintptr_t*)((uintptr_t)el + 8*(i&0x7FFF)+0x10);
        if (!e) continue;
        uintptr_t p = *(uintptr_t*)(e + 0x78*(i>>0x1C)+0x10);
        if (!p) continue;
    }
    if (!sp.empty()) {
        draw->AddText(ImVec2(10,40), ImColor(255,255,0), "Spectators:");
        for (size_t j = 0; j < sp.size(); j++)
            draw->AddText(ImVec2(10,56+j*14), ImColor(200,200,200), sp[j].c_str());
    }
}