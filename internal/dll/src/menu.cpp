#include "menu.h"
#include "config.h"
#include "auth.h"
#include <algorithm>
#include <cmath>

RageAimbotConfig g_RageConfig;
LegitAimbotConfig g_LegitConfig;
AntiAimConfig g_AAConfig;
VisualsConfig g_VisualsConfig;
MovementConfig g_MovementConfig;
MiscConfig g_MiscConfig;

int g_CurrentTab = 0;
bool g_MenuOpen = true;
bool g_Watermark = true;

static const char* tabs[] = {
    "RAGE", "LEGIT", "AA", "VISUALS", "MOVEMENT", "MISC", "CONFIG", "AUTH"
};
static const int tabCount = IM_ARRAYSIZE(tabs);

static bool showConfigPopup = false;
static char configNameBuf[128] = "";
static int configListIndex = -1;
static std::vector<std::string> configList;

static void RefreshConfigList() {
    configList = ConfigManager::ListConfigs();
    if (configListIndex >= (int)configList.size())
        configListIndex = -1;
}

static ImU32 RainbowColor(float speed, float offset, int alpha) {
    float t = ImGui::GetTime() * speed + offset;
    float r = 0.5f + 0.5f * sinf(t);
    float g = 0.5f + 0.5f * sinf(t + 2.094f);
    float b = 0.5f + 0.5f * sinf(t + 4.188f);
    return IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), alpha);
}

static void CategoryHeader(const char* label) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImU32 barColor = RainbowColor(0.6f, 0.f, 200);
    draw->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 3, pos.y + ImGui::GetTextLineHeight() + 8), barColor, 1.5f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 0.8f), label);
    ImGui::Separator();
}

static void DrawRageTab() {
    CategoryHeader("Aimbot");
    ImGui::Checkbox("Enabled", &g_RageConfig.enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Auto Shoot", &g_RageConfig.autoShoot);
    ImGui::SameLine();
    ImGui::Checkbox("Auto Scope", &g_RageConfig.autoScope);
    ImGui::Checkbox("Silent Aim", &g_RageConfig.silent);
    ImGui::SameLine();
    ImGui::Checkbox("Visible Only", &g_RageConfig.visibleOnly);
    ImGui::SameLine();
    ImGui::Checkbox("Multipoint", &g_RageConfig.multipoint);
    CategoryHeader("Accuracy");
    ImGui::SliderFloat("FOV", &g_RageConfig.fov, 0.f, 180.f, "%.1f");
    ImGui::SliderFloat("Hitchance", &g_RageConfig.hitchance, 0.f, 100.f, "%.1f%%");
    ImGui::SliderInt("Min Damage", &g_RageConfig.minDamage, 1, 200);
    ImGui::SliderFloat("Multipoint Scale", &g_RageConfig.multipointScale, 0.f, 1.f, "%.2f");
    ImGui::Combo("Hitbox Priority", &g_RageConfig.hitboxPriority, "Head\0Neck\0Chest\0Pelvis\0Auto\0");
    CategoryHeader("Special");
    ImGui::Checkbox("Jump Shot", &g_RageConfig.jumpShot);
    ImGui::SameLine();
    ImGui::Checkbox("Double Tap", &g_RageConfig.doubleTap);
    ImGui::SameLine();
    ImGui::Checkbox("Deathmatch Mode", &g_RageConfig.deathmatchMode);
}

static void DrawLegitTab() {
    CategoryHeader("Aimbot");
    ImGui::Checkbox("Enabled", &g_LegitConfig.enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Visible Only", &g_LegitConfig.visibleOnly);
    ImGui::SameLine();
    ImGui::Checkbox("Aim Lock", &g_LegitConfig.aimLock);
    ImGui::SliderFloat("FOV", &g_LegitConfig.fov, 0.f, 50.f, "%.1f");
    ImGui::SliderFloat("Smoothing", &g_LegitConfig.smoothing, 0.f, 20.f, "%.1f");
    ImGui::Combo("Hitbox", &g_LegitConfig.hitbox, "Head\0Neck\0Chest\0Stomach\0");
    if (g_LegitConfig.aimLock)
        ImGui::SliderFloat("Aim Lock Time", &g_LegitConfig.aimLockTime, 0.1f, 5.f, "%.1fs");
    CategoryHeader("Recoil Control");
    ImGui::Checkbox("RCS", &g_LegitConfig.rcs);
    if (g_LegitConfig.rcs)
        ImGui::SliderFloat("RCS Amount", &g_LegitConfig.rcsAmount, 0.f, 100.f, "%.1f%%");
    CategoryHeader("Triggerbot");
    ImGui::Checkbox("Triggerbot", &g_LegitConfig.triggerbot);
    if (g_LegitConfig.triggerbot)
        ImGui::SliderInt("Trigger Delay (ms)", &g_LegitConfig.triggerDelay, 0, 500);
}

static void DrawAATab() {
    CategoryHeader("Angles");
    ImGui::Checkbox("Enabled", &g_AAConfig.enabled);
    ImGui::Combo("Pitch", &g_AAConfig.pitch, "Off\0Down\0Up\0Zero\0Jitter\0");
    ImGui::Combo("Yaw", &g_AAConfig.yaw, "Off\0Backward\0Sideways\0Jitter\0Spin\0Custom\0");
    if (g_AAConfig.yaw == 4)
        ImGui::SliderFloat("Spin Speed", &g_AAConfig.spinSpeed, 1.f, 180.f, "%.1f");
    ImGui::SliderInt("Yaw Offset", &g_AAConfig.yawOffset, -180, 180);
    ImGui::Combo("Roll", &g_AAConfig.roll, "Off\0Zero\0Down\0180\0");
    ImGui::Checkbox("At Target", &g_AAConfig.atTarget);
    CategoryHeader("Desync (Neverloss)");
    ImGui::Checkbox("Desync", &g_AAConfig.desync);
    if (g_AAConfig.desync) {
        ImGui::SliderFloat("Amount", &g_AAConfig.desyncAmount, 0.f, 60.f, "%.1f");
        ImGui::Combo("Direction", &g_AAConfig.desyncDir, "Left\0Right\0");
    }
    CategoryHeader("Fake Latency");
    ImGui::Checkbox("Fake Lag", &g_AAConfig.fakelag);
    if (g_AAConfig.fakelag) {
        ImGui::SliderInt("Limit", &g_AAConfig.fakelagLimit, 1, 14);
        ImGui::Checkbox("On Move Only", &g_AAConfig.fakelagOnMove);
    }
}

static void DrawVisualsTab() {
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Box", &g_VisualsConfig.box);
    if (g_VisualsConfig.box)
        ImGui::Combo("Box Type", &g_VisualsConfig.boxType, "2D\0Corner\03D\0");
    ImGui::Checkbox("Health Bar", &g_VisualsConfig.healthBar);
    ImGui::Checkbox("Armor Bar", &g_VisualsConfig.armorBar);
    ImGui::Checkbox("Name", &g_VisualsConfig.name);
    ImGui::Checkbox("Weapon", &g_VisualsConfig.weapon);
    ImGui::Checkbox("Ammo", &g_VisualsConfig.ammo);
    ImGui::Checkbox("Distance", &g_VisualsConfig.distance);
    ImGui::Checkbox("Snaplines", &g_VisualsConfig.snaplines);
    ImGui::NextColumn();
    ImGui::Checkbox("Glow", &g_VisualsConfig.glow);
    if (g_VisualsConfig.glow) {
        ImGui::SliderFloat("Glow Width", &g_VisualsConfig.glowWidth, 0.f, 10.f, "%.1f");
        ImGui::SliderFloat("Glow Alpha", &g_VisualsConfig.glowAlpha, 0.f, 1.f, "%.2f");
    }
    ImGui::Checkbox("Chams", &g_VisualsConfig.chams);
    if (g_VisualsConfig.chams) {
        ImGui::Combo("Chams Type", &g_VisualsConfig.chamsType, "Normal\0Flat\0Textured\0Wireframe\0");
        ImGui::Checkbox("XQZ", &g_VisualsConfig.chamsXqz);
    }
    ImGui::Checkbox("Dlights", &g_VisualsConfig.dlights);
    ImGui::Checkbox("Hitmarker", &g_VisualsConfig.hitmarker);
    ImGui::Checkbox("Tracers", &g_VisualsConfig.tracers);
    ImGui::Checkbox("Grenade Prediction", &g_VisualsConfig.grenadePrediction);
    ImGui::Checkbox("Bomb Timer", &g_VisualsConfig.bombTimer);
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Text("Colors");
    ImGui::ColorEdit4("Box Color", (float*)&g_VisualsConfig.boxColor, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit4("Glow Color", (float*)&g_VisualsConfig.glowColor, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit4("Visible", (float*)&g_VisualsConfig.visibleColor, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit4("Invisible", (float*)&g_VisualsConfig.invisibleColor, ImGuiColorEditFlags_NoInputs);
    ImGui::Separator();
    ImGui::Checkbox("Third Person", &g_VisualsConfig.thirdperson);
    if (g_VisualsConfig.thirdperson)
        ImGui::SliderFloat("Distance", &g_VisualsConfig.thirdpersonDist, 30.f, 300.f, "%.0f");
    ImGui::Checkbox("FOV Changer", &g_VisualsConfig.fovChanger);
    if (g_VisualsConfig.fovChanger)
        ImGui::SliderFloat("FOV", &g_VisualsConfig.fovAmount, 10.f, 160.f, "%.0f");
    ImGui::Checkbox("Spectator List", &g_VisualsConfig.spectatorList);
    ImGui::Checkbox("Watermark", &g_Watermark);
    ImGui::Checkbox("Radar", &g_VisualsConfig.radar);
}

static void DrawMovementTab() {
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Bunny Hop", &g_MovementConfig.bunnyHop);
    ImGui::Checkbox("Auto Strafe", &g_MovementConfig.autoStrafe);
    if (g_MovementConfig.autoStrafe)
        ImGui::Combo("Strafe Mode", &g_MovementConfig.autoStrafeMode, "Legit\0Rage\0Directional\0");
    ImGui::Checkbox("Fast Stop", &g_MovementConfig.fastStop);
    ImGui::Checkbox("Fast Duck", &g_MovementConfig.fastDuck);
    ImGui::NextColumn();
    ImGui::Checkbox("Edge Jump", &g_MovementConfig.edgeJump);
    ImGui::Checkbox("Edge Bug", &g_MovementConfig.edgeBug);
    ImGui::Checkbox("Long Jump", &g_MovementConfig.longJump);
    ImGui::Checkbox("Jump Bug", &g_MovementConfig.jumpBug);
    ImGui::Checkbox("Crouch Bhop", &g_MovementConfig.crouchBhop);
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Checkbox("Slide Walk", &g_MovementConfig.slideWalk);
    ImGui::SameLine();
    ImGui::Checkbox("Moon Walk", &g_MovementConfig.moonWalk);
    ImGui::SameLine();
    ImGui::Checkbox("No Duck Cooldown", &g_MovementConfig.noDuckCooldown);
}

static void DrawMiscTab() {
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Radar Hack", &g_MiscConfig.radar);
    ImGui::Checkbox("No Flash", &g_MiscConfig.noFlash);
    if (g_MiscConfig.noFlash)
        ImGui::SliderFloat("Flash Reduction", &g_MiscConfig.flashReduction, 0.f, 255.f, "%.0f");
    ImGui::Checkbox("Show Impacts", &g_MiscConfig.showImpacts);
    ImGui::Checkbox("Auto Accept", &g_MiscConfig.autoAccept);
    ImGui::Checkbox("Auto Pistol", &g_MiscConfig.autoPistol);
    ImGui::Checkbox("No Smoke", &g_MiscConfig.noSmoke);
    ImGui::NextColumn();
    ImGui::Checkbox("Rank Reveal", &g_MiscConfig.rankReveal);
    ImGui::Checkbox("Reveal Money", &g_MiscConfig.revealMoney);
    ImGui::Checkbox("Voice Chat", &g_MiscConfig.voiceChat);
    ImGui::Checkbox("Unlock Inventory", &g_MiscConfig.unlockInventory);
    ImGui::Checkbox("Clan Tag", &g_MiscConfig.clantag);
    if (g_MiscConfig.clantag) {
        static char tagBuf[32] = "CAMUS";
        ImGui::InputText("Tag Text", tagBuf, sizeof(tagBuf));
        g_MiscConfig.clantagText = tagBuf;
    }
    ImGui::Columns(1);
}

static void LoadRagePreset() {
    g_RageConfig = {};
    g_RageConfig.enabled = true;
    g_RageConfig.autoShoot = true;
    g_RageConfig.autoScope = true;
    g_RageConfig.silent = true;
    g_RageConfig.fov = 180.f;
    g_RageConfig.hitchance = 100.f;
    g_RageConfig.minDamage = 1;
    g_RageConfig.hitboxPriority = 4;
    g_RageConfig.multipoint = true;
    g_RageConfig.multipointScale = 1.0f;
    g_RageConfig.visibleOnly = false;
    g_RageConfig.deathmatchMode = true;
    g_RageConfig.jumpShot = true;
    g_RageConfig.doubleTap = true;

    g_AAConfig = {};
    g_AAConfig.enabled = true;
    g_AAConfig.pitch = 1;
    g_AAConfig.yaw = 4;
    g_AAConfig.spinSpeed = 30.f;
    g_AAConfig.fakelag = true;
    g_AAConfig.fakelagLimit = 14;
    g_AAConfig.atTarget = true;
    g_AAConfig.yawOffset = 0;
    g_AAConfig.desync = true;
    g_AAConfig.desyncAmount = 58.f;
    g_AAConfig.desyncDir = 0;

    g_VisualsConfig.box = true;
    g_VisualsConfig.healthBar = true;
    g_VisualsConfig.name = true;
    g_VisualsConfig.weapon = true;
    g_VisualsConfig.snaplines = true;
    g_VisualsConfig.thirdperson = true;
    g_VisualsConfig.thirdpersonDist = 100.f;

    g_MovementConfig.bunnyHop = true;
    g_MovementConfig.autoStrafe = true;
}

static void DrawConfigTab() {
    ImGui::Text("Config Manager");
    ImGui::Separator();
    if (ImGui::Button("Load Rage Preset (HVH)")) {
        LoadRagePreset();
    }
    ImGui::Separator();
    ImGui::InputText("Config Name", configNameBuf, sizeof(configNameBuf));
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        if (strlen(configNameBuf) > 0) {
            ConfigManager::SaveConfig(configNameBuf);
            RefreshConfigList();
            configNameBuf[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        ConfigManager::Refresh();
        RefreshConfigList();
    }
    ImGui::Separator();
    if (configList.empty())
        RefreshConfigList();
    if (!configList.empty()) {
        ImGui::ListBox("Configs", &configListIndex,
            [](void* data, int idx, const char** out_text) -> bool {
                auto& vec = *(std::vector<std::string>*)data;
                *out_text = vec[idx].c_str();
                return true;
            }, &configList, (int)configList.size(), 6);
        if (configListIndex >= 0 && configListIndex < (int)configList.size()) {
            if (ImGui::Button("Load")) {
                ConfigManager::LoadConfig(configList[configListIndex].c_str());
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                ConfigManager::DeleteConfig(configList[configListIndex].c_str());
                RefreshConfigList();
            }
        }
    }
}

static void DrawAuthTab() {
    ImGui::Text("Authentication");
    ImGui::Separator();
    if (ImGui::Button("Check Auth")) {
        CheckAuth();
    }
    ImGui::SameLine();
    if (ImGui::Button("Enter Key")) {
        PromptForKey();
    }
    ImGui::Separator();
    ImGui::TextWrapped("Your HWID is bound to your subscription key. Contact support if you need to transfer your key.");
}

void Menu::Render() {
    if (!g_MenuOpen) return;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    ImGui::SetNextWindowSize(ImVec2(780, 540), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(screenSize.x * 0.5f - 390, screenSize.y * 0.5f - 270), ImGuiCond_Once);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    // Colors
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.3f, 0.7f, 1.f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.3f, 0.7f, 1.f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.5f, 0.8f, 1.f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.8f, 0.3f));

    ImGui::Begin("CAMUS CS2 CHEAT", &g_MenuOpen,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    float winWidth = ImGui::GetWindowWidth();
    float winHeight = ImGui::GetWindowHeight();

    // ── TITLE BAR ──────────────────────────────────────────────
    {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pMin = ImGui::GetWindowPos();
        ImVec2 pMax = ImVec2(pMin.x + winWidth, pMin.y + 52);
        draw->AddRectFilled(pMin, pMax, IM_COL32(8, 8, 16, 255), 8.f, ImDrawFlags_RoundCornersTop);

        // gradient glow
        float t = ImGui::GetTime();
        for (int i = 0; i < 3; i++) {
            float phase = t * 0.5f + i * 2.094f;
            float cx = pMin.x + winWidth * (0.5f + 0.5f * sinf(phase));
            draw->AddRectFilledMultiColor(
                ImVec2(cx - 120, pMax.y - 1), ImVec2(cx + 120, pMax.y + 1),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 191, 255, 180),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 0));
        }

        ImU32 titleColor = RainbowColor(0.7f, 0.f, 255);
        const char* title = "CAMUS";
        const char* subtitle = "CS2 CHEAT";
        ImVec2 titleSize = ImGui::CalcTextSize(title);
        ImVec2 subtitleSize = ImGui::CalcTextSize(subtitle);
        float totalW = titleSize.x + 6 + subtitleSize.x;
        ImVec2 textPos = ImVec2(pMin.x + winWidth * 0.5f - totalW * 0.5f, pMin.y + 14);

        for (int i = 3; i >= 0; i--) {
            int a = (int)(30.f * (float)(3 - i));
            draw->AddText(ImVec2(textPos.x, textPos.y - i), IM_COL32(0, 150, 255, a), title);
            draw->AddText(ImVec2(textPos.x + i, textPos.y), IM_COL32(150, 0, 255, a), title);
        }
        draw->AddText(textPos, titleColor, title);
        draw->AddText(ImVec2(textPos.x + titleSize.x + 6, textPos.y + 2), IM_COL32(150, 150, 200, 180), subtitle);

        ImGui::SetCursorPos(ImVec2(winWidth - 30, 13));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0, 0, 0.3f));
        if (ImGui::Button("X", ImVec2(22, 22)))
            g_MenuOpen = false;
        ImGui::PopStyleColor(2);
    }

    // ── TAB BAR ────────────────────────────────────────────────
    {
        ImGui::SetCursorPos(ImVec2(0, 52));
        float tabWidth = 97.f;
        float tabBarWidth = tabWidth * tabCount;
        float slideStart = (winWidth - tabBarWidth) * 0.5f;
        if (slideStart < 0.f) slideStart = 0.f;

        // animate active tab indicator
        static float animTarget = 0.f;
        float tgt = (float)g_CurrentTab * tabWidth;
        animTarget += (tgt - animTarget) * 0.12f;

        for (int i = 0; i < tabCount; i++) {
            ImVec2 tabPos = ImVec2(ImGui::GetWindowPos().x + slideStart + i * tabWidth, ImGui::GetWindowPos().y + 52);
            ImVec2 tabSize = ImVec2(tabWidth, 30);
            bool hovered = ImGui::IsMouseHoveringRect(tabPos, ImVec2(tabPos.x + tabSize.x, tabPos.y + tabSize.y));
            bool active = (g_CurrentTab == i);

            ImU32 bgColor = active ? IM_COL32(15, 15, 30, 255)
                         : hovered ? IM_COL32(25, 25, 50, 255)
                                   : IM_COL32(8, 8, 20, 255);

            ImGui::GetWindowDrawList()->AddRectFilled(tabPos, ImVec2(tabPos.x + tabSize.x, tabPos.y + tabSize.y), bgColor);

            ImU32 textColor = active ? IM_COL32(255, 255, 255, 255)
                           : hovered ? IM_COL32(200, 200, 255, 200)
                                     : IM_COL32(120, 120, 160, 200);
            ImVec2 textSize = ImGui::CalcTextSize(tabs[i]);
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(tabPos.x + tabWidth * 0.5f - textSize.x * 0.5f,
                       tabPos.y + tabSize.y * 0.5f - textSize.y * 0.5f),
                textColor, tabs[i]);

            if (hovered && ImGui::IsMouseClicked(0))
                g_CurrentTab = i;
        }

        // sliding underline
        float barX = ImGui::GetWindowPos().x + slideStart + animTarget;
        float barW = tabWidth - 20;
        ImU32 barColor = RainbowColor(0.5f, 0.f, 220);
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(barX + 10, ImGui::GetWindowPos().y + 52 + 28),
            ImVec2(barX + 10 + barW, ImGui::GetWindowPos().y + 52 + 30),
            barColor, 2.f);
    }

    // ── TAB CONTENT ────────────────────────────────────────────
    {
        ImGui::SetCursorPos(ImVec2(10, 92));
        ImGui::BeginChild("TabContent", ImVec2(winWidth - 20, winHeight - 104), false,
            ImGuiWindowFlags_AlwaysUseWindowPadding);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));

        switch (g_CurrentTab) {
            case 0: DrawRageTab(); break;
            case 1: DrawLegitTab(); break;
            case 2: DrawAATab(); break;
            case 3: DrawVisualsTab(); break;
            case 4: DrawMovementTab(); break;
            case 5: DrawMiscTab(); break;
            case 6: DrawConfigTab(); break;
            case 7: DrawAuthTab(); break;
        }

        ImGui::PopStyleColor();
        ImGui::EndChild();
    }

    ImGui::End();
    ImGui::PopStyleVar(5);
    ImGui::PopStyleColor(6);
}

void Menu::RenderWatermark() {
    if (!g_Watermark) return;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("Watermark", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar);
    ImGui::SetWindowPos(ImVec2(8, 8), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(0, 0));
    float time = ImGui::GetTime();
    float r = 0.5f + 0.5f * sinf(time * 0.5f);
    float g = 0.5f + 0.5f * sinf(time * 0.5f + 2.094f);
    float b = 0.5f + 0.5f * sinf(time * 0.5f + 4.188f);
    ImGui::TextColored(ImVec4(r, g, b, 1.f), "CAMUS CS2 | camuscheat.com");
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}
