#pragma once
#include <string>
#include <vector>

namespace ConfigManager {
    bool SaveConfig(const char* name);
    bool LoadConfig(const char* name);
    std::vector<std::string> ListConfigs();
    bool DeleteConfig(const char* name);
    bool Refresh();
}
