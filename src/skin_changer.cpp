#include "skin_changer.h"
#include "offsets.h"
#include "process.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>

SkinChanger::SkinChanger()
    : m_clientBase(0)
    , m_entitySystem(nullptr)
    , m_initialized(false)
    , m_lastWeaponCount(0)
    , m_lastAliveState(false) {
}

SkinChanger::~SkinChanger() {
    Stop();
}

bool SkinChanger::Initialize(const char* processName) {
    Stop();

    if (!m_memory.Open(processName)) {
        return false;
    }

    m_clientBase = Process::GetModuleBase(m_memory.GetProcessId(), Offsets::ClientModule);
    if (!m_clientBase) {
        m_memory.Close();
        return false;
    }

    if (!ValidateOffsets()) {
        m_memory.Close();
        return false;
    }

    m_entitySystem = new EntitySystem(m_memory);
    m_initialized = true;
    return true;
}

void SkinChanger::Stop() {
    m_initialized = false;
    delete m_entitySystem;
    m_entitySystem = nullptr;
    m_memory.Close();
}

bool SkinChanger::LoadConfig(const char* configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    SkinConfigFile config;
    std::string line;
    int currentWeaponId = -1;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }

        auto trim = [](std::string& s) {
            size_t start = 0;
            while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
            size_t end = s.size();
            while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
            s = s.substr(start, end - start);
        };
        trim(line);

        if (line.empty()) continue;

        if (line[0] == '[') {
            auto close = line.find(']');
            if (close != std::string::npos) {
                std::string section = line.substr(1, close - 1);
                if (section == "Settings") {
                    currentWeaponId = -2;
                } else {
                    try {
                        currentWeaponId = std::stoi(section);
                    } catch (...) {
                        currentWeaponId = -1;
                    }
                }
            }
            continue;
        }

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        trim(key);
        trim(value);

        if (currentWeaponId == -2) {
            if (key == "knife_id") {
                try { config.knifeIndex = std::stoi(value); }
                catch (...) {}
            } else if (key == "update_interval_ms") {
                try { config.updateIntervalMs = std::stof(value); }
                catch (...) {}
            }
        } else if (currentWeaponId >= 0) {
            SkinConfig sc{};
            sc.enabled = true;

            if (key == "paint_kit" || key == "skin") {
                try { sc.paintKit = std::stoi(value); }
                catch (...) {}
            } else if (key == "wear") {
                try { sc.wear = std::stof(value); }
                catch (...) {}
            } else if (key == "seed") {
                try { sc.seed = std::stoi(value); }
                catch (...) {}
            } else if (key == "stattrak") {
                try { sc.statTrak = std::stoi(value); }
                catch (...) {}
            } else if (key == "enabled") {
                sc.enabled = (value == "true" || value == "1" || value == "yes");
            }

            config.weaponSkins[currentWeaponId] = sc;
        }
    }

    m_config = config;
    return true;
}

void SkinChanger::Tick() {
    if (!m_initialized || !m_memory.IsOpen()) {
        return;
    }

    PlayerInfo player = m_entitySystem->GetLocalPlayer(m_clientBase);

    m_lastAliveState = player.isAlive && player.pawnAddress != 0;

    if (m_lastAliveState) {
        auto weapons = m_entitySystem->GetWeapons(player, m_clientBase);
        m_lastWeaponCount = static_cast<int>(weapons.size());

        if (!weapons.empty()) {
            ApplySkins();
        }
    } else {
        m_lastWeaponCount = 0;
    }
}

bool SkinChanger::IsConnected() const {
    return m_initialized && m_memory.IsOpen() && m_clientBase != 0;
}

bool SkinChanger::IsPlayerAlive() const {
    return m_lastAliveState;
}

int SkinChanger::GetWeaponCount() const {
    return m_lastWeaponCount;
}

std::vector<WeaponInfo> SkinChanger::GetCurrentWeapons() {
    if (!m_initialized || !m_memory.IsOpen()) {
        return {};
    }

    PlayerInfo player = m_entitySystem->GetLocalPlayer(m_clientBase);
    if (!player.pawnAddress || !player.isAlive) {
        return {};
    }

    return m_entitySystem->GetWeapons(player, m_clientBase);
}

void SkinChanger::ApplySkins() {
    PlayerInfo player = m_entitySystem->GetLocalPlayer(m_clientBase);
    if (!player.pawnAddress || !player.isAlive) {
        return;
    }

    auto weapons = m_entitySystem->GetWeapons(player, m_clientBase);
    if (weapons.empty()) {
        return;
    }

    for (const auto& weapon : weapons) {
        if (!weapon.entityAddress || weapon.itemDefinitionIndex < 0) {
            continue;
        }

        const SkinConfig* config = nullptr;
        auto it = m_config.weaponSkins.find(weapon.itemDefinitionIndex);
        if (it != m_config.weaponSkins.end()) {
            config = &it->second;
        }

        if (!config || !config->enabled || config->paintKit == 0) {
            continue;
        }

        m_memory.Write<int>(weapon.entityAddress + Offsets::m_iItemIDHigh, -1);
        m_memory.Write<int>(weapon.entityAddress + Offsets::m_nFallbackPaintKit, config->paintKit);
        m_memory.Write<float>(weapon.entityAddress + Offsets::m_flFallbackWear, config->wear);
        m_memory.Write<int>(weapon.entityAddress + Offsets::m_nFallbackSeed, config->seed);
        m_memory.Write<int>(weapon.entityAddress + Offsets::m_nFallbackStatTrak, config->statTrak);
    }
}

bool SkinChanger::ValidateOffsets() {
    if (!m_memory.IsOpen() || !m_clientBase) {
        return false;
    }

    for (int attempt = 0; attempt < 3; attempt++) {
        uintptr_t playerPtr = m_memory.Read<uintptr_t>(m_clientBase + Offsets::dwLocalPlayerPawn);
        if (playerPtr) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return false;
}
