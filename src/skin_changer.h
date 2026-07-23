#pragma once
#include "memory.h"
#include "entity.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

struct SkinConfig {
    bool enabled = true;
    int paintKit = 0;
    float wear = 0.001f;
    int seed = 0;
    int statTrak = -1;
};

struct SkinConfigFile {
    std::unordered_map<int, SkinConfig> weaponSkins;
    int knifeIndex = 500;
    float updateIntervalMs = 100.0f;
};

class SkinChanger {
public:
    SkinChanger();
    ~SkinChanger();

    SkinChanger(const SkinChanger&) = delete;
    SkinChanger& operator=(const SkinChanger&) = delete;

    bool Initialize(const char* processName);
    bool LoadConfig(const char* configPath);
    void Stop();
    bool IsInitialized() const { return m_initialized; }

    void Tick();

    bool IsConnected() const;
    bool IsPlayerAlive() const;
    int GetWeaponCount() const;

    const ProcessMemory& GetMemory() const { return m_memory; }
    uintptr_t GetClientBase() const { return m_clientBase; }
    const SkinConfigFile& GetConfig() const { return m_config; }
    SkinConfigFile& GetConfig() { return m_config; }

    std::vector<WeaponInfo> GetCurrentWeapons();

private:
    void ApplySkins();
    bool ValidateOffsets();

    ProcessMemory m_memory;
    uintptr_t m_clientBase = 0;
    SkinConfigFile m_config;
    EntitySystem* m_entitySystem = nullptr;
    bool m_initialized = false;

    int m_lastWeaponCount = 0;
    bool m_lastAliveState = false;
};
