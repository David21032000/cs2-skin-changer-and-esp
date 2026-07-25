#pragma once
#include <string>
#include <vector>
#include <imgui/imgui.h>

struct RageAimbotConfig {
    bool enabled = true;
    bool autoShoot = true;
    bool autoScope = true;
    bool silent = true;
    float fov = 30.f;
    float hitchance = 70.f;
    int minDamage = 20;
    int hitboxPriority = 0;
    bool multipoint = true;
    float multipointScale = 0.5f;
    bool visibleOnly = true;
    bool deathmatchMode = false;
    int selectedWeapon = 0;
};

struct LegitAimbotConfig {
    bool enabled = false;
    float fov = 10.f;
    float smoothness = 5.f;
    float smoothing = 5.f;
    int hitbox = 0;
    bool rcs = false;
    float rcsAmount = 50.f;
    bool triggerbot = false;
    int triggerDelay = 50;
    bool visibleOnly = true;
    bool aimLock = false;
    float aimLockTime = 1.f;
};

struct AntiAimConfig {
    bool enabled = false;
    int pitch = 0;
    int yaw = 0;
    int yawOffset = 0;
    int roll = 0;
    bool freak = false;
    bool jitter = false;
    bool spin = false;
    float spinSpeed = 30.f;
    bool edgeDetect = false;
    bool atTarget = false;
    bool fakelag = false;
    int fakelagLimit = 14;
    bool fakelagOnMove = true;
};

struct VisualsConfig {
    bool box = true;
    int boxType = 0;
    bool healthBar = true;
    bool armorBar = false;
    bool name = true;
    bool weapon = true;
    bool ammo = false;
    bool distance = false;
    bool snaplines = false;
    bool glow = true;
    bool chams = false;
    int chamsType = 0;
    bool chamsXqz = true;
    bool visibleOnly = true;
    bool dormant = true;
    float glowWidth = 4.f;
    float glowAlpha = 0.7f;
    ImColor boxColor = ImColor(0, 191, 255);
    ImColor glowColor = ImColor(148, 0, 211);
    ImColor visibleColor = ImColor(0, 255, 0);
    ImColor invisibleColor = ImColor(255, 0, 0);
    bool dlights = false;
    bool hitmarker = false;
    bool hitmarkerSound = false;
    bool tracers = false;
    float tracerDuration = 2.f;
    int tracerType = 0;
    bool thirdperson = false;
    float thirdpersonDist = 100.f;
    bool fovChanger = false;
    float fovAmount = 90.f;
    bool spectatorList = true;
    bool watermark = true;
    bool radar = false;
    float radarRange = 400.f;
    bool grenadePrediction = true;
    bool bombTimer = true;
    bool playerInfo = false;
};

struct MovementConfig {
    bool bunnyHop = true;
    bool autoStrafe = false;
    int autoStrafeMode = 0;
    bool edgeJump = false;
    bool edgeBug = false;
    bool longJump = false;
    bool ladderJump = false;
    bool jumpBug = false;
    bool fastStop = true;
    bool fastDuck = false;
    bool slideWalk = false;
    bool moonWalk = false;
    bool noDuckCooldown = false;
    bool crouchBhop = false;
    float bhopHitchance = 70.f;
};

struct MiscConfig {
    bool radar = true;
    bool revealRank = false;
    bool revealMoney = false;
    bool noFlash = false;
    float flashReduction = 200.f;
    bool showImpacts = false;
    bool autoAccept = false;
    bool autoPistol = false;
    bool quickReload = false;
    bool noSmoke = false;
    bool wireframeSmoke = false;
    bool rankReveal = false;
    bool voiceChat = false;
    bool voteReveal = false;
    bool clantag = false;
    std::string clantagText = "CAMUS";
    bool rankRevealAll = false;
    bool unlockInventory = false;
    bool buyBot = false;
    std::vector<std::string> buyItems;
    int buyBotRound = 0;
};

extern RageAimbotConfig g_RageConfig;
extern LegitAimbotConfig g_LegitConfig;
extern AntiAimConfig g_AAConfig;
extern VisualsConfig g_VisualsConfig;
extern MovementConfig g_MovementConfig;
extern MiscConfig g_MiscConfig;

extern int g_CurrentTab;
extern bool g_MenuOpen;
extern bool g_Watermark;

namespace Menu {
    void Render();
    void RenderWatermark();
}
