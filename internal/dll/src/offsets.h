#pragma once
#include <cstdint>

namespace Offsets {
    inline uintptr_t client = 0;
    inline uintptr_t engine = 0;
    inline uintptr_t schemasystem = 0;
    inline uintptr_t tier0 = 0;

    inline uintptr_t dwLocalPlayerPawn = 0;
    inline uintptr_t dwEntityList = 0;
    inline uintptr_t dwViewMatrix = 0;
    inline uintptr_t dwInputSystem = 0;
    inline uintptr_t dwForceJump = 0;
    inline uintptr_t dwForceAttack = 0;
    inline uintptr_t dwGlobalVars = 0;
    inline uintptr_t dwSensitivity = 0;
    inline uintptr_t dwMouseEnable = 0;

    namespace NetVar {
        inline constexpr int m_iHealth = 0x344;
        inline constexpr int m_iTeamNum = 0x3E7;
        inline constexpr int m_vecAbsOrigin = 0x70;
        inline constexpr int m_angEyeAngles = 0x3340;
        inline constexpr int m_fFlags = 0xC4C;
        inline constexpr int m_pWeaponServices = 0x15E0;
        inline constexpr int m_nArmorValue = 0x34C;
        inline constexpr int m_bHasHelmet = 0x350;
        inline constexpr int m_vecViewOffset = 0x108;
        inline constexpr int m_bIsScoped = 0x6D0;
        inline constexpr int m_hActiveWeapon = 0x1650;
        inline constexpr int m_iShotsFired = 0x160C;
        inline constexpr int m_aimPunchAngle = 0x15A0;
        inline constexpr int m_aimPunchAngleVel = 0x15AC;
        inline constexpr int m_weaponData = 0x120;
        inline constexpr int m_BoneArray = 0x1F8;
        inline constexpr int m_bDormant = 0xE8;
        inline constexpr int m_lifeState = 0x348;
        inline constexpr int m_iItemDefinitionIndex = 0x1BA;
    }

    namespace VIndex {
        inline constexpr int GetAllClasses = 8;
        inline constexpr int GetScreenSize = 5;
        inline constexpr int GetViewAngles = 18;
        inline constexpr int SetViewAngles = 19;
        inline constexpr int IsInGame = 26;
        inline constexpr int IsConnected = 27;
        inline constexpr int GetPlayerInfo = 8;
        inline constexpr int GetPlayerForUserID = 9;
        inline constexpr int ExecuteClientCmd = 112;
        inline constexpr int GetClientEntity = 3;
        inline constexpr int GetClientEntityFromHandle = 4;
        inline constexpr int GetHighestEntityIndex = 6;
        inline constexpr int EnableInput = 11;
        inline constexpr int ResetInputState = 12;
        inline constexpr int CreateMaterial = 83;
        inline constexpr int FindMaterial = 84;
        inline constexpr int GetModelIndex = 2;
        inline constexpr int GetModel = 1;
        inline constexpr int GetModelName = 3;
        inline constexpr int GetStudiomodel = 28;
        inline constexpr int GetName = 36;
        inline constexpr int PaintTraverse = 41;
        inline constexpr int PlaySound = 82;
        inline constexpr int DrawSetColor = 15;
        inline constexpr int DrawFilledRect = 16;
        inline constexpr int DrawOutlinedRect = 18;
        inline constexpr int DrawLine = 19;
        inline constexpr int DrawSetTextFont = 23;
        inline constexpr int DrawSetTextColor = 24;
        inline constexpr int DrawSetTextPos = 26;
        inline constexpr int DrawPrintText = 28;
        inline constexpr int CreateFont = 71;
        inline constexpr int SetFontGlyphSet = 72;
    }
}