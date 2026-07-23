#include "entity.h"
#include "offsets.h"
#include <cstring>
#include <cstdio>

EntitySystem::EntitySystem(const ProcessMemory& mem)
    : m_mem(mem) {
}

PlayerInfo EntitySystem::GetLocalPlayer(uintptr_t clientBase) const {
    PlayerInfo info{};
    info.pawnAddress = m_mem.Read<uintptr_t>(clientBase + Offsets::dwLocalPlayerPawn);
    if (!info.pawnAddress) {
        return info;
    }

    info.health = m_mem.Read<int>(info.pawnAddress + Offsets::m_iHealth);
    info.team = m_mem.Read<int>(info.pawnAddress + Offsets::m_iTeamNum);
    int lifeState = m_mem.Read<int>(info.pawnAddress + Offsets::m_lifeState);
    info.isAlive = lifeState == 0;

    return info;
}

std::vector<WeaponInfo> EntitySystem::GetWeapons(const PlayerInfo& player, uintptr_t clientBase) const {
    std::vector<WeaponInfo> weapons;
    if (!player.pawnAddress) {
        return weapons;
    }

    uintptr_t weaponServices = m_mem.Read<uintptr_t>(player.pawnAddress + Offsets::m_pWeaponServices);
    if (!weaponServices) {
        return weapons;
    }

    constexpr size_t vecSizeOffset = 0x08;

    uintptr_t weaponArrayPtr = m_mem.Read<uintptr_t>(weaponServices + Offsets::m_hMyWeapons);
    int weaponCount = m_mem.Read<int>(weaponServices + Offsets::m_hMyWeapons + vecSizeOffset);

    if (!weaponArrayPtr || weaponCount <= 0 || weaponCount > 64) {
        return weapons;
    }

    for (int i = 0; i < weaponCount; i++) {
        uint32_t handle = m_mem.Read<uint32_t>(weaponArrayPtr + i * sizeof(uint32_t));
        if (!handle || handle == 0xFFFFFFFF) {
            continue;
        }

        uintptr_t entityAddr = ResolveEntityHandle(handle, clientBase);
        if (!entityAddr) {
            continue;
        }

        int defIndex = m_mem.Read<int>(entityAddr + Offsets::m_iItemDefinitionIndex);
        if (!IsValidWeapon(defIndex)) {
            continue;
        }

        WeaponInfo w{};
        w.entityAddress = entityAddr;
        w.handle = handle;
        w.itemDefinitionIndex = defIndex;
        w.currentPaintKit = m_mem.Read<int>(entityAddr + Offsets::m_nFallbackPaintKit);
        w.currentWear = m_mem.Read<float>(entityAddr + Offsets::m_flFallbackWear);
        w.currentSeed = m_mem.Read<int>(entityAddr + Offsets::m_nFallbackSeed);
        w.currentStatTrak = m_mem.Read<int>(entityAddr + Offsets::m_nFallbackStatTrak);
        WeaponIndexToName(defIndex, w.weaponName, sizeof(w.weaponName));

        weapons.push_back(w);
    }

    return weapons;
}

uintptr_t EntitySystem::ResolveEntityHandle(uint32_t handle, uintptr_t clientBase) const {
    int entryIndex = handle & 0x7FFF;
    if (entryIndex == 0) {
        return 0;
    }

    uintptr_t entityList = m_mem.Read<uintptr_t>(clientBase + Offsets::dwEntityList);
    if (!entityList) {
        return 0;
    }

    uintptr_t entryPtr = entityList + (static_cast<uintptr_t>(entryIndex) - 1) * Offsets::EntityListEntrySize;
    uintptr_t entityPtr = m_mem.Read<uintptr_t>(entryPtr);
    return entityPtr;
}

void EntitySystem::WeaponIndexToName(int index, char* out, size_t outSize) {
    if (!out || outSize == 0) return;

    switch (index) {
        case 1: snprintf(out, outSize, "Desert Eagle"); break;
        case 2: snprintf(out, outSize, "Dual Berettas"); break;
        case 3: snprintf(out, outSize, "Five-SeveN"); break;
        case 4: snprintf(out, outSize, "Glock-18"); break;
        case 7: snprintf(out, outSize, "AK-47"); break;
        case 8: snprintf(out, outSize, "AUG"); break;
        case 9: snprintf(out, outSize, "AWP"); break;
        case 10: snprintf(out, outSize, "FAMAS"); break;
        case 11: snprintf(out, outSize, "G3SG1"); break;
        case 13: snprintf(out, outSize, "Galil AR"); break;
        case 14: snprintf(out, outSize, "M249"); break;
        case 16: snprintf(out, outSize, "M4A4"); break;
        case 17: snprintf(out, outSize, "M4A1-S"); break;
        case 19: snprintf(out, outSize, "MP5-SD"); break;
        case 23: snprintf(out, outSize, "MP7"); break;
        case 24: snprintf(out, outSize, "MP9"); break;
        case 25: snprintf(out, outSize, "Negev"); break;
        case 26: snprintf(out, outSize, "P90"); break;
        case 27: snprintf(out, outSize, "PP-Bizon"); break;
        case 28: snprintf(out, outSize, "R8 Revolver"); break;
        case 29: snprintf(out, outSize, "SG 553"); break;
        case 30: snprintf(out, outSize, "SCAR-20"); break;
        case 31: snprintf(out, outSize, "SSG 08"); break;
        case 32: snprintf(out, outSize, "Tec-9"); break;
        case 33: snprintf(out, outSize, "UMP-45"); break;
        case 34: snprintf(out, outSize, "USP-S"); break;
        case 35: snprintf(out, outSize, "XM1014"); break;
        case 36: snprintf(out, outSize, "MAC-10"); break;
        case 38: snprintf(out, outSize, "MAG-7"); break;
        case 39: snprintf(out, outSize, "Nova"); break;
        case 40: snprintf(out, outSize, "Sawed-Off"); break;
        case 41: snprintf(out, outSize, "P2000"); break;
        case 42: snprintf(out, outSize, "CZ75-Auto"); break;
        case 43: snprintf(out, outSize, "P250"); break;
        case 500: snprintf(out, outSize, "Bayonet"); break;
        case 503: snprintf(out, outSize, "Karambit"); break;
        case 505: snprintf(out, outSize, "M9 Bayonet"); break;
        case 506: snprintf(out, outSize, "Huntsman Knife"); break;
        case 507: snprintf(out, outSize, "Falchion Knife"); break;
        case 508: snprintf(out, outSize, "Bowie Knife"); break;
        case 509: snprintf(out, outSize, "Butterfly Knife"); break;
        case 512: snprintf(out, outSize, "Shadow Daggers"); break;
        case 514: snprintf(out, outSize, "Paracord Knife"); break;
        case 515: snprintf(out, outSize, "Survival Knife"); break;
        case 516: snprintf(out, outSize, "Ursus Knife"); break;
        case 517: snprintf(out, outSize, "Navaja Knife"); break;
        case 518: snprintf(out, outSize, "Nomad Knife"); break;
        case 519: snprintf(out, outSize, "Stiletto Knife"); break;
        case 520: snprintf(out, outSize, "Talon Knife"); break;
        case 521: snprintf(out, outSize, "Classic Knife"); break;
        default: snprintf(out, outSize, "Weapon(%d)", index); break;
    }
}

bool EntitySystem::IsValidWeapon(int index) {
    if (index >= 1 && index <= 63) return true;
    if (index >= 500 && index <= 521) return true;
    return false;
}
