#pragma once
#include "math.h"
#include <cstdint>

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
