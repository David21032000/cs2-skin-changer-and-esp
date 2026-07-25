#include "movement.h"
#include "menu.h"
#include "interfaces.h"
#include "math.h"
#include "offsets.h"
#include <algorithm>

#define IN_JUMP          (1 << 1)
#define IN_DUCK          (1 << 2)
#define IN_FORWARD       (1 << 3)
#define IN_BACK          (1 << 4)
#define IN_MOVELEFT      (1 << 9)
#define IN_MOVERIGHT     (1 << 10)

void Movement::Run(CUserCmd* cmd) {
    if (!cmd) return;
    uintptr_t local = Mem::Read<uintptr_t>(Offsets::dwLocalPlayerPawn);
    if (!local) return;

    if (g_MovementConfig.bunnyHop) {
        if (cmd->buttons & IN_JUMP) {
            int flags = Mem::Read<int>(local + Offsets::NetVar::m_fFlags);
            if (flags & 1)
                cmd->buttons |= IN_JUMP;
            else
                cmd->buttons &= ~IN_JUMP;
        }
    }

    if (g_MovementConfig.autoStrafe) {
        if (cmd->mousedx > 0) cmd->sidemove = 450.f;
        else if (cmd->mousedx < 0) cmd->sidemove = -450.f;
    }

    if (g_MovementConfig.fastStop) {
        if (!(cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))) {
            cmd->forwardmove = 0.f;
            cmd->sidemove = 0.f;
        }
    }
}
