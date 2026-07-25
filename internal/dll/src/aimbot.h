#pragma once
#include "math.h"
#include "interfaces.h"
#include <vector>

struct CUserCmd {
    virtual ~CUserCmd() = default;
    int commandNumber;
    int tickCount;
    Vec3 viewangles;
    Vec3 aimdirection;
    float forwardmove;
    float sidemove;
    float upmove;
    int buttons;
    char impulse;
    int weaponselect;
    int weaponsubtype;
    int randomseed;
    short mousedx;
    short mousedy;
    bool hasbeenpredicted;
    Vec3 headangles;
    Vec3 headoffset;
};

enum CSWeaponType : int {
    WEAPONTYPE_KNIFE = 0,
    WEAPONTYPE_PISTOL = 1,
    WEAPONTYPE_SUBMACHINEGUN = 2,
    WEAPONTYPE_RIFLE = 3,
    WEAPONTYPE_SHOTGUN = 4,
    WEAPONTYPE_SNIPER_RIFLE = 5,
    WEAPONTYPE_MACHINEGUN = 6,
    WEAPONTYPE_C4 = 7,
    WEAPONTYPE_PLACEHOLDER = 8,
    WEAPONTYPE_GRENADE = 9,
    WEAPONTYPE_UNKNOWN = 10
};

struct AimbotConfig {
    bool enabled = false;
    float fov = 30.f;
    float hitchance = 70.f;
    int minDamage = 20;
    bool silent = true;
    bool autoShoot = true;
    bool visibleOnly = true;
    int hitboxPriority = 0;
    float multipointScale = 0.5f;
};

struct WeaponInfo {
    float spread;
    float inaccuracy;
    float fireRate;
    CSWeaponType type;
    int maxDamage;
    int armorPenetration;
    float range;
};

namespace Aimbot {
    bool IsVisible(uintptr_t entity, const Vec3& start, const Vec3& end);
    Vec3 GetHitboxPos(uintptr_t entity, int hitbox);
    int EstimateDamage(uintptr_t entity, const Vec3& pos, WeaponInfo* wpn);
    float Hitchance(uintptr_t entity, const Vec3& pos, const Vec3& angles, WeaponInfo* wpn, CUserCmd* cmd);
    uintptr_t GetBestTarget(WeaponInfo* wpn, CUserCmd* cmd);
    void Run(CUserCmd* cmd);
}
