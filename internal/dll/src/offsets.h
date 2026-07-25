#pragma once
#include <cstdint>

namespace Offsets {

// ============================================================
// Module base addresses (resolved at runtime via pattern scan)
// ============================================================
inline uintptr_t client = 0;
inline uintptr_t engine = 0;
inline uintptr_t schemasystem = 0;
inline uintptr_t materialsystem2 = 0;
inline uintptr_t vguimatsurface = 0;

// ============================================================
// Signatures (pattern bytes)
// ============================================================
namespace Sig {
    inline constexpr const char* dwLocalPlayerPawn  = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x48";
    inline constexpr const char* dwLocalPlayerPawnMask = "xxx????xxxxx";

    inline constexpr const char* dwEntityList       = "\x48\x8B\x0D\x00\x00\x00\x00\x48\x89\x5C\x24\x00\x48\x85\xC9";
    inline constexpr const char* dwEntityListMask   = "xxx????xxx?xxx";

    inline constexpr const char* dwViewMatrix       = "\x48\x8D\x0D\x00\x00\x00\x00\x48\xC1\xE9\x20";
    inline constexpr const char* dwViewMatrixMask   = "xxx????xxxx";

    inline constexpr const char* dwGlobalVars       = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\xF3\x0F\x10\x05";
    inline constexpr const char* dwGlobalVarsMask   = "xxx????xxxx?xxxx";

    inline constexpr const char* dwForceJump        = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x0F\xB6\x40";
    inline constexpr const char* dwForceJumpMask    = "xxx????xxxx?xxx";

    inline constexpr const char* dwForceAttack      = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x0F\xB6\x40\x3C";
    inline constexpr const char* dwForceAttackMask  = "xxx????xxxx?xxxx";

    inline constexpr const char* dwSensitivity      = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\xF3\x0F\x10\x40\x28";
    inline constexpr const char* dwSensitivityMask  = "xxx????xxxx?xxxxx";

    inline constexpr const char* dwInputSystem      = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x75\x00\x48\x8B\x0D";
    inline constexpr const char* dwInputSystemMask  = "xxx????xxxx?xxx";

    inline constexpr const char* dwViewAngles       = "\x48\x8B\x05\x00\x00\x00\x00\xF3\x0F\x11\x45\x00\xF3\x0F\x11\x4D";
    inline constexpr const char* dwViewAnglesMask   = "xxx????xxxx?xxxx";

    inline constexpr const char* dwSensitivityPtr   = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\xF3\x0F\x10\x40\x28";
    inline constexpr const char* dwSensitivityPtrMask = "xxx????xxxx?xxxxx";
}

// ============================================================
// Offsets resolved at runtime via pattern scanning
// ============================================================
inline uintptr_t dwLocalPlayerPawn = 0;
inline uintptr_t dwEntityList = 0;
inline uintptr_t dwViewMatrix = 0;
inline uintptr_t dwGlobalVars = 0;
inline uintptr_t dwForceJump = 0;
inline uintptr_t dwForceAttack = 0;
inline uintptr_t dwSensitivity = 0;
inline uintptr_t dwInputSystem = 0;
inline uintptr_t dwViewAngles = 0;
inline uintptr_t dwSensitivityPtr = 0;
inline uintptr_t dwGameRules = 0;

// ============================================================
// NetVar offsets (Schema system)
// ============================================================
namespace NetVar {
    // C_BaseEntity
    inline constexpr int m_iTeamNum            = 0x3BF;
    inline constexpr int m_lifeState           = 0x3D0;
    inline constexpr int m_vecAbsOrigin        = 0x70;
    inline constexpr int m_vecOrigin           = 0x130;
    inline constexpr int m_flHealth            = 0x410;
    inline constexpr int m_flMaxHealth         = 0x414;
    inline constexpr int m_iHealth             = 0x414;
    inline constexpr int m_iMaxHealth          = 0x418;

    // C_BasePlayerPawn
    inline constexpr int m_hPlayerPawn         = 0x7EC;
    inline constexpr int m_pWeaponServices     = 0xDF0;
    inline constexpr int m_pInButtonState      = 0xDE8;
    inline constexpr int m_aimPunchAngle       = 0x14C0;
    inline constexpr int m_aimPunchAngleVel    = 0x14CC;
    inline constexpr int m_bIsScoped           = 0x14A8;
    inline constexpr int m_bIsDead             = 0x14A0;
    inline constexpr int m_flFlashDuration     = 0x14D0;
    inline constexpr int m_flFlashMaxAlpha     = 0x14D4;
    inline constexpr int m_iCompRank           = 0x14E0;
    inline constexpr int m_iPawnHealth         = 0x14F0;
    inline constexpr int m_vecVelocity         = 0x150;
    inline constexpr int m_vecViewOffset       = 0x170;
    inline constexpr int m_flEyeHeight         = 0x174;
    inline constexpr int m_bDormant            = 0xED;
    inline constexpr int m_fFlags              = 0x3E8;

    // C_CSPlayerPawnBase
    inline constexpr int m_iShotsFired         = 0x1410;
    inline constexpr int m_bIsWalking          = 0x1418;
    inline constexpr int m_flVelocityModifier  = 0x1420;
    inline constexpr int m_fAccuracyPenalty    = 0x1430;
    inline constexpr int m_bHasHelmet          = 0x1438;
    inline constexpr int m_bHasHeavyArmor      = 0x1439;
    inline constexpr int m_bHasDefuser         = 0x143A;
    inline constexpr int m_bHasDefuseKit       = 0x143B;
    inline constexpr int m_bGunGameImmunity    = 0x143C;
    inline constexpr int m_nArmorValue         = 0x1440;

    // C_WeaponCSBase
    inline constexpr int m_iClip1              = 0x1500;
    inline constexpr int m_iClip2              = 0x1504;
    inline constexpr int m_iPrimaryReserveAmmo = 0x1508;
    inline constexpr int m_iSecondaryReserveAmmo = 0x150C;
    inline constexpr int m_flNextPrimaryAttack = 0x1510;
    inline constexpr int m_flNextSecondaryAttack = 0x1514;
    inline constexpr int m_weaponMode          = 0x1518;
    inline constexpr int m_iItemDefinitionIndex = 0x15A0;
    inline constexpr int m_iEntityQuality      = 0x15A4;
    inline constexpr int m_hOwner              = 0x15B0;
    inline constexpr int m_iState              = 0x15C0;
    inline constexpr int m_zoomLevel           = 0x15C8;

    // C_WeaponCSBase (CS2 specific)
    inline constexpr int m_flRecoilIndex       = 0x15D0;
    inline constexpr int m_fLastShotTime       = 0x15D8;
    inline constexpr int m_weaponData          = 0x15E0;

    // C_BaseAttributableItem
    inline constexpr int m_iItemIDHigh         = 0x1610;
    inline constexpr int m_iItemIDLow          = 0x1614;
    inline constexpr int m_iAccountID          = 0x1618;
    inline constexpr int m_szCustomName        = 0x1620;

    // Entity list related
    inline constexpr int m_iEntityHandleOffset = 0x10;

    // Game rules
    inline constexpr int m_bFreezePeriod       = 0x20;
    inline constexpr int m_bWarmupPeriod       = 0x24;

    // Bone matrix
    inline constexpr int m_BoneArray           = 0xCC0;

    // View model
    inline constexpr int m_hViewModel          = 0x7E8;
    inline constexpr int m_dwViewAngles        = 0x14A0;
}

// ============================================================
// Function indices (VTable indices)
// ============================================================
namespace VIndex {
    // IBaseClientDLL
    inline constexpr int GetAllClasses         = 8;

    // IVEngineClient
    inline constexpr int GetScreenSize         = 5;
    inline constexpr int GetViewAngles         = 18;
    inline constexpr int SetViewAngles         = 19;
    inline constexpr int IsInGame              = 26;
    inline constexpr int IsConnected           = 27;
    inline constexpr int GetPlayerInfo         = 32;
    inline constexpr int GetPlayerForUserID    = 33;
    inline constexpr int ClientCmd             = 71;
    inline constexpr int ExecuteClientCmd      = 72;

    // IClientEntityList
    inline constexpr int GetClientEntity       = 3;
    inline constexpr int GetClientEntityFromHandle = 4;
    inline constexpr int GetHighestEntityIndex = 6;

    // IInputSystem
    inline constexpr int EnableInput           = 11;
    inline constexpr int ResetInputState       = 32;

    // IMaterialSystem
    inline constexpr int CreateMaterial        = 77;
    inline constexpr int FindMaterial          = 81;

    // IVModelInfo
    inline constexpr int GetModelIndex         = 2;
    inline constexpr int GetModel              = 3;
    inline constexpr int GetModelName          = 4;
    inline constexpr int GetStudiomodel        = 27;
    inline constexpr int GetBoneAccessor       = 28;

    // IPanel
    inline constexpr int GetName               = 2;
    inline constexpr int PaintTraverse         = 7;

    // ISurface
    inline constexpr int PlaySound             = 14;
    inline constexpr int DrawSetColor          = 20;
    inline constexpr int DrawSetColorAlpha     = 21;
    inline constexpr int DrawFilledRect        = 24;
    inline constexpr int DrawOutlinedRect      = 25;
    inline constexpr int DrawLine              = 26;
    inline constexpr int DrawSetTextFont       = 33;
    inline constexpr int DrawSetTextColor      = 35;
    inline constexpr int DrawSetTextPos        = 36;
    inline constexpr int DrawPrintText         = 39;
    inline constexpr int CreateFont            = 63;
    inline constexpr int SetFontGlyphSet       = 64;
}

} // namespace Offsets
