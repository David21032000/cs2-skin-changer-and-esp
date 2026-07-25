#include "anti_aim.h"
#include "menu.h"
#include "offsets.h"
#include "interfaces.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cfloat>

static uintptr_t GetLocalPlayer() {
    return Mem::Read<uintptr_t>(Offsets::dwLocalPlayerPawn);
}

void AntiAim::Run(CUserCmd* cmd, bool* sendPacket) {
    if (!g_AAConfig.enabled || !cmd) return;

    uintptr_t local = GetLocalPlayer();
    if (!local) return;

    Vec3& ang = cmd->viewangles;

    switch (g_AAConfig.pitch) {
        case 1: ang.x = -89.f; break;
        case 2: ang.x = 89.f; break;
        case 3: ang.x = 0.f; break;
        case 4: ang.x = ang.x > 0.f ? -89.f : 89.f; break;
        default: break;
    }

    static float spinAngle = 0.f;
    switch (g_AAConfig.yaw) {
        case 1: ang.y += 180.f; break;
        case 2: ang.y += 90.f; break;
        case 3: {
            static bool jitterFlip = false;
            jitterFlip = !jitterFlip;
            ang.y += jitterFlip ? 30.f : -30.f;
            break;
        }
        case 4: {
            spinAngle += g_AAConfig.spinSpeed;
            if (spinAngle > 180.f) spinAngle -= 360.f;
            ang.y = spinAngle;
            break;
        }
        case 5: {
            spinAngle += g_AAConfig.spinSpeed;
            ang.y = spinAngle;
            break;
        }
        default: break;
    }

    ang.y += static_cast<float>(g_AAConfig.yawOffset);
    ang.Clamp();

    if (g_AAConfig.atTarget) {
        int localTeam = Mem::Read<int>(local + Offsets::NetVar::m_iTeamNum);
        Vec3 localPos = Mem::Read<Vec3>(local + Offsets::NetVar::m_vecAbsOrigin);
        for (int i = 1; i <= 64; i++) {
            uintptr_t list = Mem::Read<uintptr_t>(Offsets::dwEntityList);
            if (!list) continue;
            uintptr_t entry = Mem::Read<uintptr_t>(list + i * 0x10);
            if (!entry) continue;
            uintptr_t entity = Mem::Read<uintptr_t>(entry + 0x10 * (i & 0x1FF));
            if (!entity) continue;
            if (Mem::Read<int>(entity + Offsets::NetVar::m_iTeamNum) == localTeam) continue;
            int health = Mem::Read<int>(entity + Offsets::NetVar::m_iHealth);
            if (health <= 0 || health > 100) continue;
            Vec3 targetPos = Mem::Read<Vec3>(entity + Offsets::NetVar::m_vecAbsOrigin);
            Vec3 angleToTarget = CalcAngle(localPos, targetPos);
            ang.y = angleToTarget.y + 180.f + static_cast<float>(g_AAConfig.yawOffset);
            break;
        }
    }

    // Desync (Neverloss): separate eye angles from body yaw
    if (g_AAConfig.desync) {
        float desyncAmount = std::min(g_AAConfig.desyncAmount, 58.f);
        float eyeYaw = ang.y;
        if (g_AAConfig.desyncDir == 0)
            eyeYaw += desyncAmount;
        else
            eyeYaw -= desyncAmount;

        Vec3 eyeAngles;
        eyeAngles.x = ang.x;
        eyeAngles.y = eyeYaw;
        eyeAngles.z = 0.f;
        eyeAngles.Clamp();

        Mem::Write<Vec3>(local + Offsets::NetVar::m_angEyeAngles, eyeAngles);
    }

    // Fakelag
    if (g_AAConfig.fakelag && sendPacket) {
        static int chokedCommands = 0;
        int targetChoke = std::min(g_AAConfig.fakelagLimit, 14);
        if (chokedCommands >= targetChoke) {
            *sendPacket = true;
            chokedCommands = 0;
        } else {
            *sendPacket = false;
            chokedCommands++;
        }
    }
}
