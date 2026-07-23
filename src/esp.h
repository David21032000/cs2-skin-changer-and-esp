#pragma once
#include "memory.h"
#include <cstdint>

struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };

struct EspConfig {
    bool enabled = true;
    bool drawBox = true;
    bool drawHealth = true;
    bool drawName = true;
    bool drawEnemyOnly = true;
};

class Esp {
public:
    void Render(const ProcessMemory& mem, uintptr_t clientBase);
    EspConfig config;

private:
    bool WorldToScreen(const float* matrix, const Vec3& world, Vec2& screen, int w, int h);
    Vec3 GetHeadPosition(const Vec3& origin, const Vec3& viewOffset);
    int GetScreenWidth() const;
    int GetScreenHeight() const;
};
