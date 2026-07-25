#pragma once
#include "math.h"
#include "interfaces.h"

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

struct LegitbotConfig {
    bool enabled = false;
    float fov = 5.f;
    float smooth = 10.f;
    bool rcs = false;
    float rcsAmount = 50.f;
    int aimKey = 0;
    bool visibleOnly = true;
    int hitbox = 0;
    bool triggerbot = false;
    float triggerDelay = 0.05f;
};

namespace Legitbot {
    void Run(CUserCmd* cmd);
}
