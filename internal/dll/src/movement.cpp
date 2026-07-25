#include "movement.h"
#include "interfaces.h"
#include "math.h"
#include "offsets.h"
#include <algorithm>

extern MovementConfig g_MovementConfig;

#define IN_ATTACK        (1 << 0)
#define IN_JUMP          (1 << 1)
#define IN_DUCK          (1 << 2)
#define IN_FORWARD       (1 << 3)
#define IN_BACK          (1 << 4)
#define IN_USE           (1 << 5)
#define IN_CANCEL        (1 << 6)
#define IN_LEFT          (1 << 7)
#define IN_RIGHT         (1 << 8)
#define IN_MOVELEFT      (1 << 9)
#define IN_MOVERIGHT     (1 << 10)
#define IN_ATTACK2       (1 << 11)
#define IN_RUN           (1 << 12)
#define IN_RELOAD        (1 << 13)
#define IN_ALT1          (1 << 14)
#define IN_ALT2          (1 << 15)
#define IN_SCORE         (1 << 16)
#define IN_SPEED         (1 << 17)
#define IN_WALK          (1 << 18)
#define IN_ZOOM          (1 << 19)
#define IN_WEAPON1       (1 << 20)
#define IN_WEAPON2       (1 << 21)
#define IN_BULLRUSH      (1 << 22)
#define IN_GRENADE1      (1 << 23)
#define IN_GRENADE2      (1 << 24)

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

void Movement::Run(void* cmdPtr) {
    auto cmd = (CUserCmd*)cmdPtr;
    if (!cmd) return;
    if (g_MovementConfig.bhop) {
        if (cmd->buttons & IN_JUMP) {
            uintptr_t local = Mem::Read<uintptr_t>(Offsets::dwLocalPlayerPawn);
            if (local && (*(int*)(local + 0x3F8) & 1))
                cmd->buttons |= IN_JUMP;
            else
                cmd->buttons &= ~IN_JUMP;
        }
    }
    if (g_MovementConfig.autoStrafe) {
        if (!(*(int*)(Offsets::dwLocalPlayerPawn + 0x3F8) & 1)) {
            if (cmd->mousedx > 0) cmd->sidemove = 450.f;
            else if (cmd->mousedx < 0) cmd->sidemove = -450.f;
        }
    }
    if (g_MovementConfig.quickStop) {
        if (!(cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))) {
            cmd->forwardmove = 0.f;
            cmd->sidemove = 0.f;
        }
    }
}