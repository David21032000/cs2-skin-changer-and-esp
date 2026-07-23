#include "esp.h"
#include "offsets.h"
#include "imgui.h"
#include <cmath>
#include <cstdio>

Vec3 Esp::GetHeadPosition(const Vec3& origin, const Vec3& viewOffset) {
    Vec3 head;
    head.x = origin.x + viewOffset.x;
    head.y = origin.y + viewOffset.y;
    head.z = origin.z + viewOffset.z;
    return head;
}

bool Esp::WorldToScreen(const float* matrix, const Vec3& world, Vec2& screen, int w, int h) {
    float wTransform =
        matrix[12] * world.x +
        matrix[13] * world.y +
        matrix[14] * world.z +
        matrix[15];

    if (wTransform < 0.01f) {
        return false;
    }

    float invW = 1.0f / wTransform;

    screen.x =
        (matrix[0] * world.x +
         matrix[1] * world.y +
         matrix[2] * world.z +
         matrix[3]) * invW;

    screen.y =
        (matrix[4] * world.x +
         matrix[5] * world.y +
         matrix[6] * world.z +
         matrix[7]) * invW;

    screen.x = static_cast<float>(w) * 0.5f + screen.x * static_cast<float>(w) * 0.5f;
    screen.y = static_cast<float>(h) * 0.5f - screen.y * static_cast<float>(h) * 0.5f;

    return true;
}

int Esp::GetScreenWidth() const {
    return static_cast<int>(ImGui::GetIO().DisplaySize.x);
}

int Esp::GetScreenHeight() const {
    return static_cast<int>(ImGui::GetIO().DisplaySize.y);
}

void Esp::Render(const ProcessMemory& mem, uintptr_t clientBase) {
    if (!config.enabled) return;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    if (screenW <= 0 || screenH <= 0) return;

    float viewMatrix[16];
    if (!mem.ReadArray(clientBase + Offsets::dwViewMatrix, viewMatrix, 16)) {
        return;
    }

    uintptr_t localPawn = mem.Read<uintptr_t>(clientBase + Offsets::dwLocalPlayerPawn);
    if (!localPawn) return;

    int localTeam = mem.Read<int>(localPawn + Offsets::m_iTeamNum);

    uintptr_t entityList = mem.Read<uintptr_t>(clientBase + Offsets::dwEntityList);
    if (!entityList) return;

    ImDrawList* draw = ImGui::GetForegroundDrawList();

    constexpr int maxEntities = 64;
    for (int i = 1; i <= maxEntities; i++) {
        uintptr_t entry = entityList + (static_cast<uintptr_t>(i) - 1) * Offsets::EntityListEntrySize;
        uintptr_t entity = mem.Read<uintptr_t>(entry);
        if (!entity || entity == localPawn) continue;

        int health = mem.Read<int>(entity + Offsets::m_iHealth);
        if (health < 1 || health > 100) continue;

        int team = mem.Read<int>(entity + Offsets::m_iTeamNum);
        if (team != 2 && team != 3) continue;

        if (config.drawEnemyOnly && team == localTeam) continue;

        Vec3 origin = mem.Read<Vec3>(entity + Offsets::m_vOldOrigin);
        Vec3 viewOffset = mem.Read<Vec3>(entity + Offsets::m_vecViewOffset);

        Vec3 headPos = GetHeadPosition(origin, viewOffset);

        Vec2 screenFeet, screenHead;
        if (!WorldToScreen(viewMatrix, origin, screenFeet, screenW, screenH)) continue;
        if (!WorldToScreen(viewMatrix, headPos, screenHead, screenW, screenH)) continue;

        float boxHeight = static_cast<float>(fabs(screenFeet.y - screenHead.y));
        float boxWidth = boxHeight * 0.5f;
        float x = screenFeet.x - boxWidth * 0.5f;
        float y = screenHead.y;

        if (config.drawBox) {
            ImU32 boxColor = (team == 2) ? IM_COL32(255, 180, 50, 255) : IM_COL32(80, 160, 255, 255);
            draw->AddRect(ImVec2(x, y), ImVec2(x + boxWidth, y + boxHeight), boxColor, 0.0f, 0, 1.5f);
        }

        if (config.drawHealth) {
            float healthPct = static_cast<float>(health) / 100.0f;
            float barHeight = boxHeight * healthPct;
            float barX = x - 5.0f;
            float barY = y + boxHeight - barHeight;

            ImU32 healthColor;
            if (healthPct > 0.6f) {
                healthColor = IM_COL32(50, 220, 50, 255);
            } else if (healthPct > 0.3f) {
                healthColor = IM_COL32(220, 220, 50, 255);
            } else {
                healthColor = IM_COL32(220, 50, 50, 255);
            }

            draw->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + 3.0f, y + boxHeight), IM_COL32(0, 0, 0, 180));
            draw->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + 3.0f, y + boxHeight), healthColor);
        }

        if (config.drawName) {
            const char* label = (team == 2) ? "T" : "CT";
            char text[32];
            snprintf(text, sizeof(text), "%s [%d]", label, health);
            ImVec2 textSize = ImGui::CalcTextSize(text);
            draw->AddText(
                ImVec2(x + boxWidth * 0.5f - textSize.x * 0.5f, y - textSize.y - 2.0f),
                IM_COL32(255, 255, 255, 255), text);
        }
    }
}
