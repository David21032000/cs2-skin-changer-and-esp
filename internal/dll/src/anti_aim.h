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

struct AntiAimConfig {
    bool enabled = false;
    int pitchMode = 0;
    int yawMode = 0;
    float spinSpeed = 15.f;
    float jitterRange = 30.f;
    bool desync = false;
    float desyncAmount = 58.f;
    int desyncDir = 0;
    bool fakelag = false;
    int fakelagAmount = 7;
    bool fakelagVariance = false;
};

namespace AntiAim {
    void Run(CUserCmd* cmd);
}
