#include "aimbot.h"
#include <algorithm>
#include <vector>
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

static int GetWeaponIndex(uintptr_t weapon) {
    if (!weapon) return -1;
    return Mem::Read<int>(weapon + Offsets::NetVar::m_iItemDefinitionIndex);
}

static WeaponInfo GetWeaponInfo(uintptr_t weapon) {
    WeaponInfo info = {};
    if (!weapon) return info;
    uintptr_t weaponData = Mem::Read<uintptr_t>(weapon + Offsets::NetVar::m_weaponData);
    if (!weaponData) return info;
    info.spread = Mem::Read<float>(weaponData + 0x1C);
    info.inaccuracy = Mem::Read<float>(weaponData + 0x20);
    info.fireRate = Mem::Read<float>(weaponData + 0x28);
    info.type = static_cast<CSWeaponType>(Mem::Read<int>(weaponData + 0x38));
    info.maxDamage = Mem::Read<int>(weaponData + 0x40);
    info.armorPenetration = Mem::Read<int>(weaponData + 0x44);
    info.range = Mem::Read<float>(weaponData + 0x48);
    return info;
}

static uintptr_t GetActiveWeapon(uintptr_t player) {
    if (!player) return 0;
    uintptr_t weaponServices = Mem::Read<uintptr_t>(player + Offsets::NetVar::m_pWeaponServices);
    if (!weaponServices) return 0;
    uintptr_t activeWeaponHandle = Mem::Read<uintptr_t>(weaponServices + 0xC0);
    if (!activeWeaponHandle) return 0;
    return reinterpret_cast<uintptr_t>(Interfaces::entityList->GetClientEntityFromHandle(activeWeaponHandle));
}

bool Aimbot::IsVisible(uintptr_t entity, const Vec3& start, const Vec3& end) {
    Vec3 delta = end - start;
    float dist = delta.Length();
    if (dist < 0.1f) return true;
    Vec3 dir = delta / dist;
    float step = 5.f;
    int steps = static_cast<int>(dist / step);
    for (int i = 0; i <= steps; i++) {
        Vec3 point = start + dir * (step * i);
    }
    return true;
}

static std::vector<Vec3> GetMultipoints(uintptr_t entity, int hitbox, float scale) {
    std::vector<Vec3> points;
    Vec3 center = Aimbot::GetHitboxPos(entity, hitbox);
    points.push_back(center);
    uintptr_t boneArray = Mem::Read<uintptr_t>(entity + Offsets::NetVar::m_BoneArray);
    if (!boneArray) return points;
    auto getBonePos = [&](int bone) -> Vec3 {
        return *reinterpret_cast<Vec3*>(boneArray + bone * 0x20);
    };
    int hitboxSet = Mem::Read<int>(entity + 0x10DC);
    uintptr_t studiohdr = reinterpret_cast<uintptr_t>(Interfaces::modelInfo->GetStudiomodel(
        Interfaces::modelInfo->GetModel(Mem::Read<int>(entity + 0x780))));
    if (!studiohdr) return points;
    uintptr_t hitboxSetAddr = studiohdr + 0x34;
    int numHitboxes = Mem::Read<int>(hitboxSetAddr + 0x4);
    uintptr_t hitboxArr = Mem::Read<uintptr_t>(hitboxSetAddr + 0x8);
    if (hitbox < 0 || hitbox >= numHitboxes || !hitboxArr) return points;
    uintptr_t hb = hitboxArr + hitbox * 0x3C;
    int boneIndex = Mem::Read<int>(hb + 0x14);
    Vec3 bbMin = *reinterpret_cast<Vec3*>(hb + 0x0);
    Vec3 bbMax = *reinterpret_cast<Vec3*>(hb + 0xC);
    Vec3 bonePos = getBonePos(boneIndex);
    Matrix3x4 boneMat;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            boneMat.data[i][j] = *reinterpret_cast<float*>(boneArray + boneIndex * 0x20 + i * 4 + j * 4);
    auto transform = [&](const Vec3& p) -> Vec3 {
        Vec3 out;
        out.x = p.x * boneMat[0][0] + p.y * boneMat[0][1] + p.z * boneMat[0][2] + boneMat[0][3];
        out.y = p.x * boneMat[1][0] + p.y * boneMat[1][1] + p.z * boneMat[1][2] + boneMat[1][3];
        out.z = p.x * boneMat[2][0] + p.y * boneMat[2][1] + p.z * boneMat[2][2] + boneMat[2][3];
        return out;
    };
    Vec3 centerOffset = (bbMin + bbMax) * 0.5f;
    Vec3 size = (bbMax - bbMin) * 0.5f;
    points.push_back(transform(Vec3(bbMin.x * scale + centerOffset.x * (1 - scale), bbMin.y, bbMin.z)));
    points.push_back(transform(Vec3(bbMax.x * scale + centerOffset.x * (1 - scale), bbMax.y, bbMax.z)));
    points.push_back(transform(Vec3(bbMin.x, bbMin.y * scale + centerOffset.y * (1 - scale), bbMin.z)));
    points.push_back(transform(Vec3(bbMax.x, bbMax.y * scale + centerOffset.y * (1 - scale), bbMax.z)));
    return points;
}

Vec3 Aimbot::GetHitboxPos(uintptr_t entity, int hitbox) {
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
    Vec3 bonePos = *reinterpret_cast<Vec3*>(boneArray + boneIndex * 0x20);
    Vec3 out;
    float* mat = reinterpret_cast<float*>(boneArray + boneIndex * 0x20);
    out.x = center.x * mat[0] + center.y * mat[1] + center.z * mat[2] + mat[3];
    out.y = center.x * mat[4] + center.y * mat[5] + center.z * mat[6] + mat[7];
    out.z = center.x * mat[8] + center.y * mat[9] + center.z * mat[10] + mat[11];
    return out;
}

int Aimbot::EstimateDamage(uintptr_t entity, const Vec3& pos, WeaponInfo* wpn) {
    if (!entity || !wpn) return 0;
    Vec3 localPos = Mem::Read<Vec3>(GetLocalPlayer() + Offsets::NetVar::m_vecAbsOrigin);
    Vec3 localEye = localPos + Mem::Read<Vec3>(GetLocalPlayer() + Offsets::NetVar::m_vecViewOffset);
    float dist = localEye.DistTo(pos);
    if (dist > wpn->range) return 0;
    float dmgMult = 1.f - (dist / wpn->range) * 0.5f;
    int rawDmg = static_cast<int>(wpn->maxDamage * dmgMult);
    int health = Mem::Read<int>(entity + Offsets::NetVar::m_iHealth);
    int armor = Mem::Read<int>(entity + Offsets::NetVar::m_nArmorValue);
    bool hasHelmet = Mem::Read<bool>(entity + Offsets::NetVar::m_bHasHelmet);
    if (armor > 0) {
        float armorRatio = wpn->armorPenetration / 100.f;
        int dmgAfterArmor = static_cast<int>(rawDmg * armorRatio);
        int armorDamage = (rawDmg - dmgAfterArmor) / 2;
        if (armorDamage > armor) armorDamage = armor;
        rawDmg = dmgAfterArmor;
    }
    return std::min(rawDmg, health);
}

static int GetHitboxPriority(int mode) {
    switch (mode) {
        case 0: return 6;
        case 1: return 7;
        case 2: return 5;
        case 3: return 5;
        default: return 6;
    }
}

static float GetFovToTarget(const Vec3& localEye, const Vec3& targetPos, const Vec3& angles) {
    Vec3 forward;
    AngleVectors(angles, &forward);
    Vec3 toTarget = (targetPos - localEye).Normalized();
    float dot = forward.Dot(toTarget);
    return RAD2DEG(std::acos(std::clamp(dot, -1.f, 1.f)));
}

float Aimbot::Hitchance(uintptr_t entity, const Vec3& pos, const Vec3& angles, WeaponInfo* wpn, CUserCmd* cmd) {
    if (!entity || !wpn || !cmd) return 0.f;
    Vec3 localPos = Mem::Read<Vec3>(GetLocalPlayer() + Offsets::NetVar::m_vecAbsOrigin);
    Vec3 localEye = localPos + Mem::Read<Vec3>(GetLocalPlayer() + Offsets::NetVar::m_vecViewOffset);
    Vec3 forward;
    AngleVectors(angles, &forward);
    float totalSpread = wpn->spread + wpn->inaccuracy;
    float spreadRad = DEG2RAD(totalSpread);
    Vec3 dirToTarget = (pos - localEye).Normalized();
    float dist = localEye.DistTo(pos);
    float maxAngle = std::asin(std::min(spreadRad, 0.99f));
    float targetAngle = std::acos(std::clamp(forward.Dot(dirToTarget), -1.f, 1.f));
    if (targetAngle > maxAngle) return 0.f;
    Vec3 right = forward.Cross(Vec3(0, 0, 1)).Normalized();
    Vec3 up = right.Cross(forward).Normalized();
    int hits = 0;
    int total = 256;
    int seed = cmd->randomseed;
    for (int i = 0; i < total; i++) {
        float r1 = ((seed + i * 2 + 1) & 0x7FFFFFFF) / 2147483648.f;
        float r2 = ((seed + i * 2 + 2) & 0x7FFFFFFF) / 2147483648.f;
        float theta = r1 * 2.f * M_PI;
        float phi = std::acos(2.f * r2 - 1.f);
        Vec3 spreadVec;
        spreadVec.x = std::sin(phi) * std::cos(theta) * spreadRad;
        spreadVec.y = std::sin(phi) * std::sin(theta) * spreadRad;
        spreadVec.z = std::cos(phi) * spreadRad;
        Vec3 shotDir = forward + right * spreadVec.x + up * spreadVec.y;
        Vec3 shotDirNorm = shotDir.Normalized();
        float shotFrac = dist / (shotDirNorm.Length() * dist);
        Vec3 hitPos = localEye + shotDirNorm * (dist / std::max(shotDirNorm.Dot(dirToTarget), 0.1f));
        Vec3 targetCenter = GetHitboxPos(entity, 6);
        if (hitPos.DistTo(targetCenter) < 15.f) hits++;
    }
    return (float)hits / (float)total * 100.f;
}

uintptr_t Aimbot::GetBestTarget(WeaponInfo* wpn, CUserCmd* cmd) {
    uintptr_t local = GetLocalPlayer();
    if (!local) return 0;
    int localTeam = Mem::Read<int>(local + Offsets::NetVar::m_iTeamNum);
    Vec3 localPos = Mem::Read<Vec3>(local + Offsets::NetVar::m_vecAbsOrigin);
    Vec3 localEye = localPos + Mem::Read<Vec3>(local + Offsets::NetVar::m_vecViewOffset);
    float bestFov = std::numeric_limits<float>::max();
    uintptr_t bestTarget = 0;
    for (int i = 1; i <= 64; i++) {
        uintptr_t entity = GetEntityFromList(i);
        if (!entity) continue;
        if (Mem::Read<int>(entity + Offsets::NetVar::m_iTeamNum) == localTeam) continue;
        int health = Mem::Read<int>(entity + Offsets::NetVar::m_iHealth);
        if (health <= 0 || health > 100) continue;
        if (Mem::Read<bool>(entity + Offsets::NetVar::m_bDormant)) continue;
        if (!IsVisible(entity, localEye, GetHitboxPos(entity, 6))) continue;
        Vec3 targetPos = GetHitboxPos(entity, GetHitboxPriority(0));
        float fov = GetFovToTarget(localEye, targetPos, cmd->viewangles);
        if (fov > 180.f) continue;
        if (fov < bestFov) {
            bestFov = fov;
            bestTarget = entity;
        }
    }
    return bestTarget;
}

void Aimbot::Run(CUserCmd* cmd) {
    static AimbotConfig cfg;
    if (!cfg.enabled || !cmd) return;
    uintptr_t local = GetLocalPlayer();
    if (!local) return;
    uintptr_t weapon = GetActiveWeapon(local);
    if (!weapon) return;
    WeaponInfo wpn = GetWeaponInfo(weapon);
    uintptr_t target = GetBestTarget(&wpn, cmd);
    if (!target) return;
    Vec3 localPos = Mem::Read<Vec3>(local + Offsets::NetVar::m_vecAbsOrigin);
    Vec3 localEye = localPos + Mem::Read<Vec3>(local + Offsets::NetVar::m_vecViewOffset);
    int priority = GetHitboxPriority(cfg.hitboxPriority);
    Vec3 aimPos = GetHitboxPos(target, priority);
    if (cfg.multipointScale > 0.f) {
        auto points = GetMultipoints(target, priority, cfg.multipointScale);
        float bestDmg = 0.f;
        Vec3 bestPos = aimPos;
        for (auto& pt : points) {
            if (!IsVisible(target, localEye, pt)) continue;
            int dmg = EstimateDamage(target, pt, &wpn);
            if (dmg > bestDmg && dmg >= cfg.minDamage) {
                bestDmg = static_cast<float>(dmg);
                bestPos = pt;
            }
        }
        if (bestDmg > 0.f) aimPos = bestPos;
    }
    Vec3 targetAngles = CalcAngle(localEye, aimPos);
    float hitchance = Hitchance(target, aimPos, targetAngles, &wpn, cmd);
    if (hitchance < cfg.hitchance) return;
    Vec3 current = cmd->viewangles;
    Vec3 delta = targetAngles - current;
    delta.Clamp();
    if (cfg.silent) {
        cmd->viewangles = targetAngles;
    }
    if (cfg.autoShoot && hitchance >= cfg.hitchance) {
        cmd->buttons |= 1;
    }
}
