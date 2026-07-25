#pragma once

#include <cstdint>
#include <string>

struct VisualsConfig {
    bool enabled = true;
    bool box = true;
    bool health = true;
    bool armor = true;
    bool name = true;
    bool weapon = true;
    bool snaplines = false;
    bool enemiesOnly = true;
    bool visibleOnly = true;
    bool glow = false;
    bool chamsFlat = false;
    bool chamsMetallic = false;
    bool chamsWireframe = false;
    bool worldModulation = false;
};

namespace Visuals {
    void Render();
    void Chams(void* ctx, void* state, void* info, void* data);
}