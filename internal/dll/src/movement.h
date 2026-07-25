#pragma once

struct MovementConfig {
    bool bhop = true;
    bool autoStrafe = true;
    bool edgeJump = false;
    bool quickStop = false;
    int autoStrafeMode = 0;
};

namespace Movement {
    void Run(void* cmd);
}
