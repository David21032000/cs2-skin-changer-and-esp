#pragma once
#include <cstdint>

// Update offsets from: https://github.com/a2x/cs2-dumper
// Dump: client.dll -> dump.cs
// Search: dwLocalPlayerPawn, dwEntityList, etc.

namespace Offsets {

inline constexpr const char* ProcessName = "cs2.exe";
inline constexpr const char* ClientModule = "client.dll";

inline constexpr uintptr_t dwLocalPlayerPawn = 0x1878B00;
inline constexpr uintptr_t dwEntityList = 0x1A5E6A8;
inline constexpr uintptr_t dwViewMatrix = 0x1A5D350;

inline constexpr uintptr_t m_pWeaponServices = 0x1280;
inline constexpr uintptr_t m_hMyWeapons = 0x48;
inline constexpr uintptr_t m_iItemDefinitionIndex = 0x1E;
inline constexpr uintptr_t m_iItemIDHigh = 0x1610;
inline constexpr uintptr_t m_nFallbackPaintKit = 0x1620;
inline constexpr uintptr_t m_flFallbackWear = 0x161C;
inline constexpr uintptr_t m_nFallbackSeed = 0x1628;
inline constexpr uintptr_t m_nFallbackStatTrak = 0x1624;
inline constexpr uintptr_t m_iAccountID = 0x1630;
inline constexpr uintptr_t m_vOldOrigin = 0x1324;
inline constexpr uintptr_t m_vecViewOffset = 0xCB0;
inline constexpr uintptr_t m_iTeamNum = 0x3E3;
inline constexpr uintptr_t m_iHealth = 0x344;
inline constexpr uintptr_t m_lifeState = 0x358;

inline constexpr size_t EntityListEntrySize = 0x78;

} // namespace Offsets
