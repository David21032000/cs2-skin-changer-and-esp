#include "ui.h"
#include "imgui.h"
#include <cstdio>
#include <cstring>

static int g_selectedWeapon = -1;
static char g_skinIdBuf[16] = "";
static char g_wearBuf[16] = "";
static char g_seedBuf[16] = "";
static char g_stattrakBuf[16] = "";
static char g_newWeaponId[16] = "";
static char g_newPaintKit[16] = "";

static void SelectWeapon(int weaponId, const SkinConfig& config) {
    g_selectedWeapon = weaponId;
    snprintf(g_skinIdBuf, sizeof(g_skinIdBuf), "%d", config.paintKit);
    snprintf(g_wearBuf, sizeof(g_wearBuf), "%.3f", config.wear);
    snprintf(g_seedBuf, sizeof(g_seedBuf), "%d", config.seed);
    snprintf(g_stattrakBuf, sizeof(g_stattrakBuf), "%d", config.statTrak);
}

static void SaveWeaponConfig(SkinChanger& changer) {
    if (g_selectedWeapon < 0) return;

    auto& config = changer.GetConfig();
    SkinConfig sc;
    sc.paintKit = std::atoi(g_skinIdBuf);
    sc.wear = static_cast<float>(std::atof(g_wearBuf));
    sc.seed = std::atoi(g_seedBuf);
    sc.statTrak = std::atoi(g_stattrakBuf);

    auto it = config.weaponSkins.find(g_selectedWeapon);
    if (it != config.weaponSkins.end()) {
        sc.enabled = it->second.enabled;
    }

    config.weaponSkins[g_selectedWeapon] = sc;
}

static void RenderEspTab(EspConfig& esp) {
    ImGui::Checkbox("Enable ESP", &esp.enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Shows player boxes, health bars, and names through walls.");

    if (!esp.enabled) {
        ImGui::BeginDisabled();
    }

    ImGui::Indent(16.0f);
    ImGui::Checkbox("Draw Box", &esp.drawBox);
    ImGui::Checkbox("Draw Health Bar", &esp.drawHealth);
    ImGui::Checkbox("Draw Name", &esp.drawName);
    ImGui::Checkbox("Enemies Only", &esp.drawEnemyOnly);
    ImGui::Unindent(16.0f);

    if (!esp.enabled) {
        ImGui::EndDisabled();
    }
}

static void RenderWeaponTable(SkinChanger& changer) {
    auto& config = changer.GetConfig();

    if (config.weaponSkins.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "No skins configured. Add a weapon below.");
        return;
    }

    const float tableHeight = ImGui::GetContentRegionAvail().y - 120.0f;

    if (!ImGui::BeginTable("weapons", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp,
            ImVec2(0.0f, (tableHeight > 100.0f) ? tableHeight : 200.0f)))
    {
        return;
    }

    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24.0f);
    ImGui::TableSetupColumn("Weapon", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Skin ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Wear", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    ImGui::TableHeadersRow();

    for (auto& [weaponId, skinCfg] : config.weaponSkins) {
        ImGui::PushID(weaponId);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Checkbox("##en", &skinCfg.enabled);

        ImGui::TableSetColumnIndex(1);
        char name[64];
        EntitySystem::WeaponIndexToName(weaponId, name, sizeof(name));

        bool isSelected = (g_selectedWeapon == weaponId);
        if (ImGui::Selectable(name, isSelected,
                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                ImVec2(0, 0)))
        {
            SelectWeapon(weaponId, skinCfg);
        }

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d", skinCfg.paintKit);

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.3f", skinCfg.wear);

        ImGui::PopID();
    }

    ImGui::EndTable();
}

static void RenderSelectedWeaponEditor(SkinChanger& changer) {
    if (g_selectedWeapon < 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
            "Select a weapon from the list above to edit.");
        return;
    }

    char name[64];
    EntitySystem::WeaponIndexToName(g_selectedWeapon, name, sizeof(name));

    ImGui::SeparatorText(name);

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Skin ID", g_skinIdBuf, sizeof(g_skinIdBuf));
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Skin paint kit ID number.\n"
            "Examples:\n"
            "  282 = AK-47 | Redline\n"
            "  344 = AWP | Dragon Lore\n"
            "  524 = AK-47 | Asiimov\n"
            "See skins.json for more IDs.");
    }

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Wear", g_wearBuf, sizeof(g_wearBuf));
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Wear value (0.0 to 1.0):\n"
            "  0.001 = Factory New\n"
            "  0.070 = Minimal Wear\n"
            "  0.150 = Field-Tested\n"
            "  0.380 = Well-Worn\n"
            "  0.450 = Battle-Scarred");
    }

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Seed", g_seedBuf, sizeof(g_seedBuf));
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Pattern seed (0-999). Controls the pattern for skins that have it.");
    }

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("StatTrak", g_stattrakBuf, sizeof(g_stattrakBuf));
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "StatTrak kill count.\n"
            "  -1 = No StatTrak\n"
            "   0+ = Show this many kills");
    }

    if (ImGui::Button("Save", ImVec2(100, 0))) {
        SaveWeaponConfig(changer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove", ImVec2(100, 0))) {
        auto& config = changer.GetConfig();
        config.weaponSkins.erase(g_selectedWeapon);
        g_selectedWeapon = -1;
    }
}

static void RenderAddWeaponSection(SkinChanger& changer) {
    ImGui::SeparatorText("Add Weapon");

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Weapon ID", g_newWeaponId, sizeof(g_newWeaponId));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Skin ID", g_newPaintKit, sizeof(g_newPaintKit));

    ImGui::SameLine();
    if (ImGui::Button("Add")) {
        int weaponId = std::atoi(g_newWeaponId);
        int paintKit = std::atoi(g_newPaintKit);
        if (weaponId > 0 && paintKit > 0) {
            auto& config = changer.GetConfig();
            SkinConfig sc;
            sc.paintKit = paintKit;
            config.weaponSkins[weaponId] = sc;
            SelectWeapon(weaponId, sc);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload Config")) {
        changer.LoadConfig("skins.json");
    }
}

void RenderUI(SkinChanger& changer, bool connected, EspConfig& espConfig) {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(560, 660), ImGuiCond_Once);

    ImGui::Begin("CS2 Skin Changer", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    if (connected) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "● Connected");
        ImGui::SameLine(0, 20);
        if (changer.IsPlayerAlive()) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                "Alive  (%d weapons)", changer.GetWeaponCount());
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Spectating / Dead");
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
            "● Disconnected  (waiting for CS2...)");
    }

    ImGui::Spacing();

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Skins")) {
            RenderWeaponTable(changer);
            ImGui::Spacing();
            RenderSelectedWeaponEditor(changer);
            RenderAddWeaponSection(changer);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ESP")) {
            RenderEspTab(espConfig);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About")) {
            ImGui::TextDisabled(
                "INS - Toggle menu\n"
                "Skins are applied automatically every tick.\n"
                "ESP renders when enabled, even with menu hidden.\n"
                "Offsets must be updated after each CS2 patch.\n\n"
                "Project: github.com/a2x/cs2-dumper\n");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
