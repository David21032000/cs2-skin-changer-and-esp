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
    inline constexpr int m_iTeamNum            = 0x3E7;
    inline constexpr int m_lifeState           = 0x354;
    inline constexpr int m_vecAbsOrigin        = 0x70;
    inline constexpr int m_vecOrigin           = 0x80; // CGameSceneNode::m_vecOrigin
    inline constexpr int m_iHealth             = 0x34C;
    inline constexpr int m_iMaxHealth          = 0x348;
    inline constexpr int m_pGameSceneNode      = 0x330; // CGameSceneNode*
    inline constexpr int m_fFlags              = 0x3F4;
    inline constexpr int m_bDormant            = 0xED;
    inline constexpr int m_vecVelocity         = 0x430;

    // C_BaseModelEntity
    inline constexpr int m_vecViewOffset       = 0xE78;

    // C_BasePlayerPawn
    inline constexpr int m_pWeaponServices     = 0x1208;
    inline constexpr int m_pItemServices       = 0x1210;
    inline constexpr int m_hPlayerPawn         = 0x914; // CCSPlayerController

    // C_CSPlayerPawnBase
    inline constexpr int m_flFlashDuration     = 0x1428;
    inline constexpr int m_flFlashMaxAlpha     = 0x1424;
    inline constexpr int m_flFlashOverlayAlpha = 0x141C;
    inline constexpr int m_flFlashScreenshotAlpha = 0x1418;

    // C_CSPlayerPawn
    inline constexpr int m_iShotsFired         = 0x1C84;
    inline constexpr int m_bIsWalking          = 0x1C50;
    inline constexpr int m_flVelocityModifier  = 0x1C8C;
    inline constexpr int m_bIsScoped           = 0x1C70;
    inline constexpr int m_bIsDefusing         = 0x1C72;
    inline constexpr int m_bGunGameImmunity    = 0x3258;
    inline constexpr int m_nArmorValue         = 0x1C9C;
    inline constexpr int m_ArmorValue          = 0x1C9C;
    inline constexpr int m_angEyeAngles        = 0x3340;
    inline constexpr int m_iIDEntIndex         = 0x341C;
    inline constexpr int m_aimPunchAngle       = 0x14C0;
    inline constexpr int m_aimPunchAngleVel    = 0x14CC;

    // CCSPlayer_ItemServices
    inline constexpr int m_bHasDefuser         = 0x48;
    inline constexpr int m_bHasHelmet          = 0x49;
    inline constexpr int m_bHasHeavyArmor      = 0x4A;

    // CCSPlayerController
    inline constexpr int m_iPawnHealth         = 0x920;
    inline constexpr int m_iCompRank           = 0x888;
    inline constexpr int m_bPawnIsAlive        = 0x91C;

    // C_CSWeaponBase
    inline constexpr int m_weaponMode          = 0x17D8;
    inline constexpr int m_fAccuracyPenalty    = 0x17F0;
    inline constexpr int m_flRecoilIndex       = 0x1800;
    inline constexpr int m_iRecoilIndex        = 0x17FC;

    // C_CSWeaponBaseGun
    inline constexpr int m_zoomLevel           = 0x1CE0;

    // C_BasePlayerWeapon
    inline constexpr int m_iClip1              = 0x1700;
    inline constexpr int m_iClip2              = 0x1704;
    inline constexpr int m_pReserveAmmo        = 0x1708; // int32[2]
    inline constexpr int m_iPrimaryReserveAmmo = 0x1708;
    inline constexpr int m_iSecondaryReserveAmmo = 0x170C;

    // C_EconItemView (in C_AttributeContainer::m_Item)
    inline constexpr int m_iItemDefinitionIndex = 0x1BA;
    inline constexpr int m_iEntityQuality      = 0x1BC;
    inline constexpr int m_iItemIDHigh         = 0x1D0;
    inline constexpr int m_iItemIDLow          = 0x1D4;
    inline constexpr int m_iAccountID          = 0x1D8;
    inline constexpr int m_szCustomName        = 0x1620;

    // C_BasePlayerWeapon
    inline constexpr int m_flNextPrimaryAttack = 0x16F4; // m_flNextPrimaryAttackTickRatio
    inline constexpr int m_flNextSecondaryAttack = 0x16FC; // m_flNextSecondaryAttackTickRatio

    // C_BaseEntity / misc
    inline constexpr int m_pInButtonState      = 0xDE8;
    inline constexpr int m_bIsDead             = 0x14A0;
    inline constexpr int m_flEyeHeight         = 0x174;
    inline constexpr int m_flHealth            = 0x34C;
    inline constexpr int m_flMaxHealth         = 0x348;

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

    // Misc weapon
    inline constexpr int m_hOwner              = 0x15B0;
    inline constexpr int m_iState              = 0x15C0;
    inline constexpr int m_fLastShotTime       = 0x1928;
    inline constexpr int m_weaponData          = 0x15E0;
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
