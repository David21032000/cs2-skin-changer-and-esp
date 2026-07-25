#include "config.h"
#include "menu.h"
#include "json.hpp"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::string GetConfigPath() {
    char path[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA(nullptr), path, MAX_PATH);
    std::string full = path;
    size_t pos = full.find_last_of("\\/");
    if (pos != std::string::npos)
        full = full.substr(0, pos);
    return full + "\\camus\\configs\\";
}

static json SerializeRage() {
    json j;
    RageAimbotConfig& c = g_RageConfig;
    j["enabled"] = c.enabled;
    j["autoShoot"] = c.autoShoot;
    j["autoScope"] = c.autoScope;
    j["silent"] = c.silent;
    j["fov"] = c.fov;
    j["hitchance"] = c.hitchance;
    j["minDamage"] = c.minDamage;
    j["hitboxPriority"] = c.hitboxPriority;
    j["multipoint"] = c.multipoint;
    j["multipointScale"] = c.multipointScale;
    j["visibleOnly"] = c.visibleOnly;
    j["deathmatchMode"] = c.deathmatchMode;
    j["jumpShot"] = c.jumpShot;
    j["doubleTap"] = c.doubleTap;
    j["selectedWeapon"] = c.selectedWeapon;
    return j;
}

static json SerializeLegit() {
    json j;
    LegitAimbotConfig& c = g_LegitConfig;
    j["enabled"] = c.enabled;
    j["fov"] = c.fov;
    j["smoothness"] = c.smoothness;
    j["smoothing"] = c.smoothing;
    j["hitbox"] = c.hitbox;
    j["rcs"] = c.rcs;
    j["rcsAmount"] = c.rcsAmount;
    j["triggerbot"] = c.triggerbot;
    j["triggerDelay"] = c.triggerDelay;
    j["visibleOnly"] = c.visibleOnly;
    j["aimLock"] = c.aimLock;
    j["aimLockTime"] = c.aimLockTime;
    return j;
}

static json SerializeAA() {
    json j;
    AntiAimConfig& c = g_AAConfig;
    j["enabled"] = c.enabled;
    j["pitch"] = c.pitch;
    j["yaw"] = c.yaw;
    j["yawOffset"] = c.yawOffset;
    j["roll"] = c.roll;
    j["freak"] = c.freak;
    j["jitter"] = c.jitter;
    j["spin"] = c.spin;
    j["spinSpeed"] = c.spinSpeed;
    j["edgeDetect"] = c.edgeDetect;
    j["atTarget"] = c.atTarget;
    j["fakelag"] = c.fakelag;
    j["fakelagLimit"] = c.fakelagLimit;
    j["fakelagOnMove"] = c.fakelagOnMove;
    return j;
}

static json SerializeVisuals() {
    json j;
    VisualsConfig& c = g_VisualsConfig;
    j["box"] = c.box;
    j["boxType"] = c.boxType;
    j["healthBar"] = c.healthBar;
    j["armorBar"] = c.armorBar;
    j["name"] = c.name;
    j["weapon"] = c.weapon;
    j["ammo"] = c.ammo;
    j["distance"] = c.distance;
    j["snaplines"] = c.snaplines;
    j["glow"] = c.glow;
    j["chams"] = c.chams;
    j["chamsType"] = c.chamsType;
    j["chamsXqz"] = c.chamsXqz;
    j["visibleOnly"] = c.visibleOnly;
    j["dormant"] = c.dormant;
    j["glowWidth"] = c.glowWidth;
    j["glowAlpha"] = c.glowAlpha;
    j["dlights"] = c.dlights;
    j["hitmarker"] = c.hitmarker;
    j["hitmarkerSound"] = c.hitmarkerSound;
    j["tracers"] = c.tracers;
    j["tracerDuration"] = c.tracerDuration;
    j["tracerType"] = c.tracerType;
    j["thirdperson"] = c.thirdperson;
    j["thirdpersonDist"] = c.thirdpersonDist;
    j["fovChanger"] = c.fovChanger;
    j["fovAmount"] = c.fovAmount;
    j["spectatorList"] = c.spectatorList;
    j["watermark"] = c.watermark;
    j["radar"] = c.radar;
    j["radarRange"] = c.radarRange;
    j["grenadePrediction"] = c.grenadePrediction;
    j["bombTimer"] = c.bombTimer;
    j["playerInfo"] = c.playerInfo;
    j["boxColor"] = { c.boxColor.Value.x, c.boxColor.Value.y, c.boxColor.Value.z, c.boxColor.Value.w };
    j["glowColor"] = { c.glowColor.Value.x, c.glowColor.Value.y, c.glowColor.Value.z, c.glowColor.Value.w };
    j["visibleColor"] = { c.visibleColor.Value.x, c.visibleColor.Value.y, c.visibleColor.Value.z, c.visibleColor.Value.w };
    j["invisibleColor"] = { c.invisibleColor.Value.x, c.invisibleColor.Value.y, c.invisibleColor.Value.z, c.invisibleColor.Value.w };
    return j;
}

static json SerializeMovement() {
    json j;
    MovementConfig& c = g_MovementConfig;
    j["bunnyHop"] = c.bunnyHop;
    j["autoStrafe"] = c.autoStrafe;
    j["autoStrafeMode"] = c.autoStrafeMode;
    j["edgeJump"] = c.edgeJump;
    j["edgeBug"] = c.edgeBug;
    j["longJump"] = c.longJump;
    j["ladderJump"] = c.ladderJump;
    j["jumpBug"] = c.jumpBug;
    j["fastStop"] = c.fastStop;
    j["fastDuck"] = c.fastDuck;
    j["slideWalk"] = c.slideWalk;
    j["moonWalk"] = c.moonWalk;
    j["noDuckCooldown"] = c.noDuckCooldown;
    j["crouchBhop"] = c.crouchBhop;
    j["bhopHitchance"] = c.bhopHitchance;
    return j;
}

static json SerializeMisc() {
    json j;
    MiscConfig& c = g_MiscConfig;
    j["radar"] = c.radar;
    j["revealRank"] = c.revealRank;
    j["revealMoney"] = c.revealMoney;
    j["noFlash"] = c.noFlash;
    j["flashReduction"] = c.flashReduction;
    j["showImpacts"] = c.showImpacts;
    j["autoAccept"] = c.autoAccept;
    j["autoPistol"] = c.autoPistol;
    j["quickReload"] = c.quickReload;
    j["noSmoke"] = c.noSmoke;
    j["wireframeSmoke"] = c.wireframeSmoke;
    j["rankReveal"] = c.rankReveal;
    j["voiceChat"] = c.voiceChat;
    j["voteReveal"] = c.voteReveal;
    j["clantag"] = c.clantag;
    j["clantagText"] = c.clantagText;
    j["rankRevealAll"] = c.rankRevealAll;
    j["unlockInventory"] = c.unlockInventory;
    j["buyBot"] = c.buyBot;
    j["buyBotRound"] = c.buyBotRound;
    return j;
}

static void DeserializeRage(const json& j) {
    RageAimbotConfig& c = g_RageConfig;
    if (j.contains("enabled")) c.enabled = j["enabled"];
    if (j.contains("autoShoot")) c.autoShoot = j["autoShoot"];
    if (j.contains("autoScope")) c.autoScope = j["autoScope"];
    if (j.contains("silent")) c.silent = j["silent"];
    if (j.contains("fov")) c.fov = j["fov"];
    if (j.contains("hitchance")) c.hitchance = j["hitchance"];
    if (j.contains("minDamage")) c.minDamage = j["minDamage"];
    if (j.contains("hitboxPriority")) c.hitboxPriority = j["hitboxPriority"];
    if (j.contains("multipoint")) c.multipoint = j["multipoint"];
    if (j.contains("multipointScale")) c.multipointScale = j["multipointScale"];
    if (j.contains("visibleOnly")) c.visibleOnly = j["visibleOnly"];
    if (j.contains("deathmatchMode")) c.deathmatchMode = j["deathmatchMode"];
    if (j.contains("jumpShot")) c.jumpShot = j["jumpShot"];
    if (j.contains("doubleTap")) c.doubleTap = j["doubleTap"];
    if (j.contains("selectedWeapon")) c.selectedWeapon = j["selectedWeapon"];
}

static void DeserializeLegit(const json& j) {
    LegitAimbotConfig& c = g_LegitConfig;
    if (j.contains("enabled")) c.enabled = j["enabled"];
    if (j.contains("fov")) c.fov = j["fov"];
    if (j.contains("smoothness")) c.smoothness = j["smoothness"];
    if (j.contains("smoothing")) c.smoothing = j["smoothing"];
    if (j.contains("hitbox")) c.hitbox = j["hitbox"];
    if (j.contains("rcs")) c.rcs = j["rcs"];
    if (j.contains("rcsAmount")) c.rcsAmount = j["rcsAmount"];
    if (j.contains("triggerbot")) c.triggerbot = j["triggerbot"];
    if (j.contains("triggerDelay")) c.triggerDelay = j["triggerDelay"];
    if (j.contains("visibleOnly")) c.visibleOnly = j["visibleOnly"];
    if (j.contains("aimLock")) c.aimLock = j["aimLock"];
    if (j.contains("aimLockTime")) c.aimLockTime = j["aimLockTime"];
}

static void DeserializeAA(const json& j) {
    AntiAimConfig& c = g_AAConfig;
    if (j.contains("enabled")) c.enabled = j["enabled"];
    if (j.contains("pitch")) c.pitch = j["pitch"];
    if (j.contains("yaw")) c.yaw = j["yaw"];
    if (j.contains("yawOffset")) c.yawOffset = j["yawOffset"];
    if (j.contains("roll")) c.roll = j["roll"];
    if (j.contains("freak")) c.freak = j["freak"];
    if (j.contains("jitter")) c.jitter = j["jitter"];
    if (j.contains("spin")) c.spin = j["spin"];
    if (j.contains("spinSpeed")) c.spinSpeed = j["spinSpeed"];
    if (j.contains("edgeDetect")) c.edgeDetect = j["edgeDetect"];
    if (j.contains("atTarget")) c.atTarget = j["atTarget"];
    if (j.contains("fakelag")) c.fakelag = j["fakelag"];
    if (j.contains("fakelagLimit")) c.fakelagLimit = j["fakelagLimit"];
    if (j.contains("fakelagOnMove")) c.fakelagOnMove = j["fakelagOnMove"];
}

static void DeserializeVisuals(const json& j) {
    VisualsConfig& c = g_VisualsConfig;
    if (j.contains("box")) c.box = j["box"];
    if (j.contains("boxType")) c.boxType = j["boxType"];
    if (j.contains("healthBar")) c.healthBar = j["healthBar"];
    if (j.contains("armorBar")) c.armorBar = j["armorBar"];
    if (j.contains("name")) c.name = j["name"];
    if (j.contains("weapon")) c.weapon = j["weapon"];
    if (j.contains("ammo")) c.ammo = j["ammo"];
    if (j.contains("distance")) c.distance = j["distance"];
    if (j.contains("snaplines")) c.snaplines = j["snaplines"];
    if (j.contains("glow")) c.glow = j["glow"];
    if (j.contains("chams")) c.chams = j["chams"];
    if (j.contains("chamsType")) c.chamsType = j["chamsType"];
    if (j.contains("chamsXqz")) c.chamsXqz = j["chamsXqz"];
    if (j.contains("visibleOnly")) c.visibleOnly = j["visibleOnly"];
    if (j.contains("dormant")) c.dormant = j["dormant"];
    if (j.contains("glowWidth")) c.glowWidth = j["glowWidth"];
    if (j.contains("glowAlpha")) c.glowAlpha = j["glowAlpha"];
    if (j.contains("dlights")) c.dlights = j["dlights"];
    if (j.contains("hitmarker")) c.hitmarker = j["hitmarker"];
    if (j.contains("hitmarkerSound")) c.hitmarkerSound = j["hitmarkerSound"];
    if (j.contains("tracers")) c.tracers = j["tracers"];
    if (j.contains("tracerDuration")) c.tracerDuration = j["tracerDuration"];
    if (j.contains("tracerType")) c.tracerType = j["tracerType"];
    if (j.contains("thirdperson")) c.thirdperson = j["thirdperson"];
    if (j.contains("thirdpersonDist")) c.thirdpersonDist = j["thirdpersonDist"];
    if (j.contains("fovChanger")) c.fovChanger = j["fovChanger"];
    if (j.contains("fovAmount")) c.fovAmount = j["fovAmount"];
    if (j.contains("spectatorList")) c.spectatorList = j["spectatorList"];
    if (j.contains("watermark")) c.watermark = j["watermark"];
    if (j.contains("radar")) c.radar = j["radar"];
    if (j.contains("radarRange")) c.radarRange = j["radarRange"];
    if (j.contains("grenadePrediction")) c.grenadePrediction = j["grenadePrediction"];
    if (j.contains("bombTimer")) c.bombTimer = j["bombTimer"];
    if (j.contains("playerInfo")) c.playerInfo = j["playerInfo"];
    auto loadColor = [](const json& arr) -> ImColor {
        if (arr.size() >= 4)
            return ImColor((float)arr[0], (float)arr[1], (float)arr[2], (float)arr[3]);
        return ImColor(255, 255, 255);
    };
    if (j.contains("boxColor") && j["boxColor"].is_array())
        c.boxColor = loadColor(j["boxColor"]);
    if (j.contains("glowColor") && j["glowColor"].is_array())
        c.glowColor = loadColor(j["glowColor"]);
    if (j.contains("visibleColor") && j["visibleColor"].is_array())
        c.visibleColor = loadColor(j["visibleColor"]);
    if (j.contains("invisibleColor") && j["invisibleColor"].is_array())
        c.invisibleColor = loadColor(j["invisibleColor"]);
}

static void DeserializeMovement(const json& j) {
    MovementConfig& c = g_MovementConfig;
    if (j.contains("bunnyHop")) c.bunnyHop = j["bunnyHop"];
    if (j.contains("autoStrafe")) c.autoStrafe = j["autoStrafe"];
    if (j.contains("autoStrafeMode")) c.autoStrafeMode = j["autoStrafeMode"];
    if (j.contains("edgeJump")) c.edgeJump = j["edgeJump"];
    if (j.contains("edgeBug")) c.edgeBug = j["edgeBug"];
    if (j.contains("longJump")) c.longJump = j["longJump"];
    if (j.contains("ladderJump")) c.ladderJump = j["ladderJump"];
    if (j.contains("jumpBug")) c.jumpBug = j["jumpBug"];
    if (j.contains("fastStop")) c.fastStop = j["fastStop"];
    if (j.contains("fastDuck")) c.fastDuck = j["fastDuck"];
    if (j.contains("slideWalk")) c.slideWalk = j["slideWalk"];
    if (j.contains("moonWalk")) c.moonWalk = j["moonWalk"];
    if (j.contains("noDuckCooldown")) c.noDuckCooldown = j["noDuckCooldown"];
    if (j.contains("crouchBhop")) c.crouchBhop = j["crouchBhop"];
    if (j.contains("bhopHitchance")) c.bhopHitchance = j["bhopHitchance"];
}

static void DeserializeMisc(const json& j) {
    MiscConfig& c = g_MiscConfig;
    if (j.contains("radar")) c.radar = j["radar"];
    if (j.contains("revealRank")) c.revealRank = j["revealRank"];
    if (j.contains("revealMoney")) c.revealMoney = j["revealMoney"];
    if (j.contains("noFlash")) c.noFlash = j["noFlash"];
    if (j.contains("flashReduction")) c.flashReduction = j["flashReduction"];
    if (j.contains("showImpacts")) c.showImpacts = j["showImpacts"];
    if (j.contains("autoAccept")) c.autoAccept = j["autoAccept"];
    if (j.contains("autoPistol")) c.autoPistol = j["autoPistol"];
    if (j.contains("quickReload")) c.quickReload = j["quickReload"];
    if (j.contains("noSmoke")) c.noSmoke = j["noSmoke"];
    if (j.contains("wireframeSmoke")) c.wireframeSmoke = j["wireframeSmoke"];
    if (j.contains("rankReveal")) c.rankReveal = j["rankReveal"];
    if (j.contains("voiceChat")) c.voiceChat = j["voiceChat"];
    if (j.contains("voteReveal")) c.voteReveal = j["voteReveal"];
    if (j.contains("clantag")) c.clantag = j["clantag"];
    if (j.contains("clantagText")) c.clantagText = j["clantagText"];
    if (j.contains("rankRevealAll")) c.rankRevealAll = j["rankRevealAll"];
    if (j.contains("unlockInventory")) c.unlockInventory = j["unlockInventory"];
    if (j.contains("buyBot")) c.buyBot = j["buyBot"];
    if (j.contains("buyBotRound")) c.buyBotRound = j["buyBotRound"];
}

bool ConfigManager::SaveConfig(const char* name) {
    std::string dir = GetConfigPath();
    fs::create_directories(dir);
    std::string filePath = dir + name + ".json";
    json root;
    root["rage"] = SerializeRage();
    root["legit"] = SerializeLegit();
    root["aa"] = SerializeAA();
    root["visuals"] = SerializeVisuals();
    root["movement"] = SerializeMovement();
    root["misc"] = SerializeMisc();
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;
    ofs << root.dump(4);
    return true;
}

bool ConfigManager::LoadConfig(const char* name) {
    std::string filePath = GetConfigPath() + name + ".json";
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;
    json root;
    try {
        ifs >> root;
    } catch (...) { return false; }
    if (root.contains("rage")) DeserializeRage(root["rage"]);
    if (root.contains("legit")) DeserializeLegit(root["legit"]);
    if (root.contains("aa")) DeserializeAA(root["aa"]);
    if (root.contains("visuals")) DeserializeVisuals(root["visuals"]);
    if (root.contains("movement")) DeserializeMovement(root["movement"]);
    if (root.contains("misc")) DeserializeMisc(root["misc"]);
    return true;
}

std::vector<std::string> ConfigManager::ListConfigs() {
    std::vector<std::string> result;
    std::string dir = GetConfigPath();
    if (!fs::exists(dir)) return result;
    for (auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".json") {
            result.push_back(entry.path().stem().string());
        }
    }
    return result;
}

bool ConfigManager::DeleteConfig(const char* name) {
    std::string filePath = GetConfigPath() + name + ".json";
    return fs::remove(filePath);
}

bool ConfigManager::Refresh() {
    std::string filePath = GetConfigPath() + "autoexec.json";
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;
    json root;
    try { ifs >> root; } catch (...) { return false; }
    if (root.contains("rage")) DeserializeRage(root["rage"]);
    if (root.contains("legit")) DeserializeLegit(root["legit"]);
    if (root.contains("aa")) DeserializeAA(root["aa"]);
    if (root.contains("visuals")) DeserializeVisuals(root["visuals"]);
    if (root.contains("movement")) DeserializeMovement(root["movement"]);
    if (root.contains("misc")) DeserializeMisc(root["misc"]);
    return true;
}
