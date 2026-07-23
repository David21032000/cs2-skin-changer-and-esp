#pragma once
#include <cstdint>
#include <vector>
#include <cstddef>
#include "memory.h"

struct WeaponInfo {
    uintptr_t entityAddress = 0;
    uint32_t handle = 0;
    int itemDefinitionIndex = 0;
    int currentPaintKit = 0;
    float currentWear = 0.0f;
    int currentSeed = 0;
    int currentStatTrak = 0;
    char weaponName[64]{};
};

struct PlayerInfo {
    uintptr_t pawnAddress = 0;
    int health = 0;
    int team = 0;
    bool isAlive = false;
};

class EntitySystem {
public:
    explicit EntitySystem(const ProcessMemory& mem);

    PlayerInfo GetLocalPlayer(uintptr_t clientBase) const;
    std::vector<WeaponInfo> GetWeapons(const PlayerInfo& player, uintptr_t clientBase) const;
    uintptr_t ResolveEntityHandle(uint32_t handle, uintptr_t clientBase) const;

    static void WeaponIndexToName(int index, char* out, size_t outSize);
    static bool IsValidWeapon(int index);

private:
    const ProcessMemory& m_mem;
};
