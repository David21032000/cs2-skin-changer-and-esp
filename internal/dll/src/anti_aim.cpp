#include "anti_aim.h"
#include <algorithm>
#include <cstdlib>

static uintptr_t GetLocalPlayer() {
    return Mem::Read<uintptr_t>(Offsets::client + Offsets::dwLocalPlayerPawn);
}

static bool IsAttacking(CUserCmd* cmd) {
    return (cmd->buttons & 1) != 0;
}

static bool IsMoving(CUserCmd* cmd) {
    return cmd->forwardmove != 0.f || cmd->sidemove != 0.f;
}

static int GetChokedCommands(uintptr_t player) {
    return Mem::Read<int>(player + 0xC4C);
}

static void SetChokedCommands(uintptr_t player, int value) {
    Mem::Write<int>(player + 0xC4C, value);
}

static float GetRandomFloat(float min, float max) {
    return min + (max - min) * (std::rand() / (float)RAND_MAX);
}

void AntiAim::Run(CUserCmd* cmd) {
    static AntiAimConfig cfg;
    static float spinAngle = 0.f;
    static bool jitterFlip = false;
    static int fakelagAccum = 0;

    if (!cfg.enabled || !cmd) return;
    if (IsAttacking(cmd)) return;

    uintptr_t local = GetLocalPlayer();
    if (!local) return;

    Vec3& ang = cmd->viewangles;

    switch (cfg.pitchMode) {
        case 1: ang.x = -89.f; break;
        case 2: ang.x = 89.f; break;
        default: break;
    }

    switch (cfg.yawMode) {
        case 1: {
            spinAngle += cfg.spinSpeed;
            if (spinAngle > 180.f) spinAngle -= 360.f;
            ang.y = spinAngle;
            break;
        }
        case 2: {
            jitterFlip = !jitterFlip;
            ang.y += jitterFlip ? cfg.jitterRange : -cfg.jitterRange;
            break;
        }
        case 3: {
            ang.y += 180.f;
            break;
        }
        case 4: {
            ang.y += 90.f;
            break;
        }
        case 5: {
            ang.y += 90.f;
            break;
        }
        default: break;
    }

    if (cfg.yawMode == 0) {
        ang.z = 0.f;
    }

    ang.Clamp();

    if (cfg.desync) {
        float desyncAng = std::min(cfg.desyncAmount, 58.f);
        if (cfg.desyncDir == 0) {
            ang.y -= desyncAng;
        } else {
            ang.y += desyncAng;
        }
        ang.Clamp();
        uintptr_t playerPawn = local;
        if (playerPawn) {
            float eyeYaw = ang.y;
            Mem::Write<float>(playerPawn + 0x14B0, eyeYaw);
        }
    }

    if (cfg.fakelag) {
        int targetChoke = cfg.fakelagAmount;
        if (cfg.fakelagVariance) {
            targetChoke -= static_cast<int>(GetRandomFloat(0, 3));
            if (targetChoke < 1) targetChoke = 1;
        }
        int choked = GetChokedCommands(local);
        if (choked < targetChoke) {
            SetChokedCommands(local, choked + 1);
            cmd->commandNumber = cmd->commandNumber;
        } else {
            SetChokedCommands(local, 0);
        }
    }
}
