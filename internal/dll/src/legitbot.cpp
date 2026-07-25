#include "legitbot.h"
#include "menu.h"
#include <algorithm>
#include <cstdlib>
#include <limits>

static uintptr_t GetLocalPlayer() {
    return Mem::Read<uintptr_t>(Offsets::dwLocalPlayerPawn);
}

static uintptr_t GetEntityFromList(int index) {
    uintptr_t list = Mem::Read<uintptr_t>(Offsets::dwEntityList);
    if (!list) return 0;
    uintptr_t entry = Mem::Read<uintptr_t>(list + index * 0x10);
    if (!entry) return 0;
    return Mem::Read<uintptr_t>(entry + 0x10 * (index & 0x1FF));
}

static uintptr_t GetActiveWeapon(uintptr_t player) {
    if (!player) return 0;
    uintptr_t weaponServices = Mem::Read<uintptr_t>(player + Offsets::NetVar::m_pWeaponServices);
    if (!weaponServices) return 0;
    uintptr_t activeWeaponHandle = Mem::Read<uintptr_t>(weaponServices + 0xC0);
    if (!activeWeaponHandle) return 0;
    return reinterpret_cast<uintptr_t>(Interfaces::entityList->GetClientEntityFromHandle(activeWeaponHandle));
}

static Vec3 GetHitboxPos(uintptr_t entity, int hitbox) {
    uintptr_t boneArray = Mem::Read<uintptr_t>(entity + Offsets::NetVar::m_BoneArray);
    if (!boneArray) return {};
    uintptr_t studiohdr = reinterpret_cast<uintptr_t>(Interfaces::modelInfo->GetStudiomodel(
        Interfaces::modelInfo->GetModel(Mem::Read<int>(entity + 0x780))));
    if (!studiohdr) return {};
    uintptr_t hitboxSetAddr = studiohdr + 0x34;
    int numHitboxes = Mem::Read<int>(hitboxSetAddr + 0x4);
    uintptr_t hitboxArr = Mem::Read<uintptr_t>(hitboxSetAddr + 0x8);
    if (hitbox < 0 || hitbox >= numHitboxes || !hitboxArr) return {};
    uintptr_t hb = hitboxArr + hitbox * 0x3C;
    int boneIndex = Mem::Read<int>(hb + 0x14);
    Vec3 bbMin = *reinterpret_cast<Vec3*>(hb + 0x0);
    Vec3 bbMax = *reinterpret_cast<Vec3*>(hb + 0xC);
    Vec3 center = (bbMin + bbMax) * 0.5f;
    float* mat = reinterpret_cast<float*>(boneArray + boneIndex * 0x20);
    Vec3 out;
    out.x = center.x * mat[0] + center.y * mat[1] + center.z * mat[2] + mat[3];
    out.y = center.x * mat[4] + center.y * mat[5] + center.z * mat[6] + mat[7];
    out.z = center.x * mat[8] + center.y * mat[9] + center.z * mat[10] + mat[11];
    return out;
}

static bool IsKeyDown(int key) {
    if (key == 0) return true;
    if (key == 1) return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
    if (key == 2) return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (key == 3) return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    if (key == 4) return (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    return true;
}

static Vec3 GetPunchAngles(uintptr_t local) {
    return Mem::Read<Vec3>(local + Offsets::NetVar::m_aimPunchAngle);
}

static float GetFovToTarget(const Vec3& localEye, const Vec3& targetPos, const Vec3& angles) {
    Vec3 forward;
    AngleVectors(angles, &forward);
    Vec3 toTarget = (targetPos - localEye).Normalized();
    float dot = forward.Dot(toTarget);
    return RAD2DEG(std::acos(std::clamp(dot, -1.f, 1.f)));
}

void Legitbot::Run(CUserCmd* cmd) {
    if (!g_LegitConfig.enabled || !cmd) return;
    uintptr_t local = GetLocalPlayer();
    if (!local) return;
    int localTeam = Mem::Read<int>(local + Offsets::NetVar::m_iTeamNum);
    Vec3 localPos = Mem::Read<Vec3>(local + Offsets::NetVar::m_vecAbsOrigin);
    Vec3 localEye = localPos + Mem::Read<Vec3>(local + Offsets::NetVar::m_vecViewOffset);
    bool keyHeld = IsKeyDown(0);
    if (!keyHeld && !g_LegitConfig.triggerbot) return;
    Vec3 punch = {};
    if (g_LegitConfig.rcs && keyHeld) {
        punch = GetPunchAngles(local) * (g_LegitConfig.rcsAmount / 100.f);
    }
    uintptr_t bestTarget = 0;
    float bestFov = g_LegitConfig.fov;
    Vec3 bestPos;
    for (int i = 1; i <= 64; i++) {
        uintptr_t entity = GetEntityFromList(i);
        if (!entity) continue;
        if (Mem::Read<int>(entity + Offsets::NetVar::m_iTeamNum) == localTeam) continue;
        int health = Mem::Read<int>(entity + Offsets::NetVar::m_iHealth);
        if (health <= 0 || health > 100) continue;
        if (Mem::Read<bool>(entity + Offsets::NetVar::m_bDormant)) continue;
        Vec3 targetPos = GetHitboxPos(entity, g_LegitConfig.hitbox);
        float fov = GetFovToTarget(localEye, targetPos, cmd->viewangles);
        if (fov > bestFov) continue;
        bestTarget = entity;
        bestPos = targetPos;
        bestFov = fov;
    }
    if (!bestTarget || !keyHeld) return;
    Vec3 aimAngles = CalcAngle(localEye, bestPos);
    aimAngles.x -= punch.x;
    aimAngles.y -= punch.y;
    aimAngles.Clamp();
    Vec3 current = cmd->viewangles;
    Vec3 delta = aimAngles - current;
    delta.Clamp();
    float smoothFactor = g_LegitConfig.smoothing > 1.f ? 1.f / g_LegitConfig.smoothing : 1.f;
    float randomFactor = ((std::rand() % 100) / 100.f - 0.5f) * 0.1f;
    smoothFactor += randomFactor;
    if (smoothFactor > 1.f) smoothFactor = 1.f;
    if (smoothFactor < 0.05f) smoothFactor = 0.05f;
    cmd->viewangles.x = current.x + delta.x * smoothFactor;
    cmd->viewangles.y = current.y + delta.y * smoothFactor;
    cmd->viewangles.Clamp();
}
