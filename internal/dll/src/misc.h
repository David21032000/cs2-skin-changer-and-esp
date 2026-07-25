#pragma once
#include <string>

struct MiscConfig {
    bool thirdPerson = false;
    float tpDistance = 100.0f;
    bool radar = true;
    bool watermark = true;
    bool spectatorList = true;
    bool hitmarker = false;
    bool clantag = false;
    std::string clantagText = "CAMUS";
};

namespace Misc {
    void Run(void* cmd);
    void RenderWatermark();
    void SpectatorList();
}
