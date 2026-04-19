#include "pch.h"
#include "Menu.h"
#include "imgui.h"
#include "GameAPI.h"
#include "Offsets.h"
#include "Console.h"
#include "ModManager.h"

namespace Menu {
    bool bShowMenu = false;
}
static bool bShowConsole = true;
static bool bThemeApplied = false;

static void ApplyCustomTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 8);
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.ChildRounding = 4.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.50f, 0.45f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.60f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.40f, 0.35f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.35f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.45f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.28f, 0.38f, 1.00f);
    
    colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
}

static void DrawWatermark() {
    ImGuiIO& io = ImGui::GetIO();
    ImU32 textColor = IM_COL32(220, 220, 220, 220);
    ImU32 accentColor = IM_COL32(80, 220, 180, 255);

    ImGui::GetBackgroundDrawList()->AddText(
        ImVec2(14.0f, io.DisplaySize.y - 32.0f),
        textColor,
        "CTW Mod Loader v1.0"
    );
    ImGui::GetBackgroundDrawList()->AddText(
        ImVec2(14.0f, io.DisplaySize.y - 16.0f),
        accentColor,
        "INSERT - Menu | F6 - Reload Mods | END - Exit"
    );
}

static void DrawStatusPanel(uintptr_t moduleBase) {
    ImGui::TextWrapped("Craft The World mod loader now features a sleek UI and quick access to functions.");
    ImGui::Spacing();
    ImGui::Text("Status:");
    ImGui::Separator();

    void* currentWorld = *(void**)(moduleBase + Offsets::offsetWorldInstance);
    ImGui::BulletText("Version: %s", "v1.0");
    ImGui::BulletText("Base: 0x%p", (void*)moduleBase);
    ImGui::BulletText("World: %s", currentWorld ? "Loaded" : "Not loaded");
    ImGui::BulletText("Menu: %s", Menu::bShowMenu ? "Open" : "Hidden");
}

static void DrawModsPanel() {
    auto& mods = ModManager::GetInstance().GetMods();
    if (mods.empty()) {
        ImGui::TextDisabled("No mods loaded. Create something in the mods/ folder.");
        return;
    }
    
    static int selectedMod = 0;
    if (selectedMod >= mods.size()) {
        selectedMod = mods.size() - 1;
    }
    if (selectedMod < 0 && !mods.empty()) {
        selectedMod = 0;
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.18f, 1.00f));
    ImGui::BeginChild("ModsList", ImVec2(200, 220), true);
    for (int i = 0; i < mods.size(); i++) {
        const auto& mod = mods[i];
        
        bool isSelected = (selectedMod == i);
        std::string label = mod.name;
        if (!mod.enabled) {
            label += " (Off)";
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        }

        if (ImGui::Selectable(label.c_str(), isSelected)) {
            selectedMod = i;
        }

        if (!mod.enabled) {
            ImGui::PopStyleColor();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::BeginChild("ModDetails", ImVec2(0, 220), true);
    if (selectedMod >= 0 && selectedMod < mods.size()) {
        const auto& mod = mods[selectedMod];
        ImGui::TextColored(ImVec4(0.40f, 0.85f, 0.65f, 1.0f), "%s", mod.name.c_str());
        ImGui::TextDisabled("ID: %s   |   Version: %s", mod.id.c_str(), mod.version.c_str());
        
        if (!mod.author.empty()) {
            ImGui::Text("Author: %s", mod.author.c_str());
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("%s", mod.description.empty() ? "No description available." : mod.description.c_str());

        ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 45.0f);
        ImGui::Separator();
        ImGui::Spacing();

        if (mod.enabled) {
            if (ImGui::Button("Disable", ImVec2(100, 24))) {
                ModManager::GetInstance().ToggleMod(mod.id);
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.50f, 0.20f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.60f, 0.25f, 1.0f));
            if (ImGui::Button("Enable", ImVec2(100, 24))) {
                ModManager::GetInstance().ToggleMod(mod.id);
            }
            ImGui::PopStyleColor(2);
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(100, 24))) {
            ModManager::GetInstance().DeleteMod(mod.id);
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::TextDisabled("(Press F6 to restart)");
    } else {
        ImGui::TextDisabled("Select a mod to view details");
    }
    ImGui::EndChild();
}

static void DrawFooter() {
    ImGui::Separator();
    ImGui::TextDisabled("Craft The World | Mod Loader");
    ImGui::SameLine();
    ImGui::TextDisabled("by CTW Moodloader");
}

void Menu::Toggle() {
    Menu::bShowMenu = !Menu::bShowMenu;
}

void Menu::Draw() {
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);
    DrawWatermark();

    if (!Menu::bShowMenu) {
        return;
    }

    if (!bThemeApplied) {
        ApplyCustomTheme();
        bThemeApplied = true;
    }

    ImGui::SetNextWindowSize(ImVec2(700, 480), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("CTW Mod Loader", &Menu::bShowMenu, windowFlags)) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.40f, 0.35f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.50f, 0.45f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.15f, 0.30f, 0.25f, 1.00f));

        ImGui::TextColored(ImVec4(0.40f, 0.85f, 0.65f, 1.0f), "Craft The World Mod Loader");
        ImGui::TextUnformatted("Game & Mods Management");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        DrawStatusPanel(moduleBase);
        ImGui::Spacing();
        DrawModsPanel();
        ImGui::Spacing();
        ImGui::Checkbox("Show console", &bShowConsole);

        ImGui::Spacing();
        DrawFooter();
        ImGui::PopStyleColor(3);
    }
    ImGui::End();

    if (bShowConsole) {
        Console::Draw("Developer Console", &bShowConsole);
    }
}