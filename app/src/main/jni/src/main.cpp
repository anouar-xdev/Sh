#include "main.h"
#include "ImGui/FONTS/MAIN_FONT.h"
#include "ImGui/FONTS/ICONS.h"
#include "ImGui/FONTS/SPECIAL.h"
#include "ImGui/FONTS/font.h"

#include <iomanip>
#include <map>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>
#include <thread>
#include <cmath>
#include <chrono>
#include <linux/input.h>

#include "ENC/oxorany_include.h"
#include "ENC/oxorany.h"
#include "driver.h"

float g_ThemeColorArr[4] = { 1.00f, 0.00f, 0.00f, 1.00f };
#define g_ThemeColor ImVec4(g_ThemeColorArr[0], g_ThemeColorArr[1], g_ThemeColorArr[2], g_ThemeColorArr[3])

float g_GlowIntensity = 1.0f;
float g_MenuSize = 1400.0f;
float g_MenuWidth = 1400.0f;
float g_MenuHeight = 820.0f;
bool g_darkMode = false;
int currentTab = 0;
bool showMenu = true;
int g_menuMode = 1;

ImFont* g_IconFont = nullptr;
ImFont* g_BoldFont = nullptr;

#include "Scarecrow/hook.h"

struct SwitchState {
    float knobPosition;
    bool targetState;
    bool isAnimating;
    std::chrono::steady_clock::time_point lastUpdate;
};

std::unordered_map<ImGuiID, SwitchState> g_SwitchStates;

bool ModernToggle(const char* label, bool* v) {
    ImGuiWindow* w = ImGui::GetCurrentWindow();
    if (w->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = w->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    float toggle_height = ImGui::GetFrameHeight() * 0.72f;
    float toggle_width = toggle_height * 1.85f;
    float knob_size = toggle_height * 0.82f;

    const ImVec2 pos = w->DC.CursorPos;
    float width = ImGui::GetContentRegionAvail().x;

    ImRect bb(pos, ImVec2(pos.x + width, pos.y + toggle_height + g.Style.ItemInnerSpacing.y));

    ImGui::ItemSize(bb, g.Style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    auto& state = g_SwitchStates[id];
    auto now = std::chrono::steady_clock::now();

    if (state.lastUpdate.time_since_epoch().count() == 0) {
        state.knobPosition = *v ? 1.0f : 0.0f;
        state.targetState = *v;
        state.isAnimating = false;
        state.lastUpdate = now;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    if (pressed) {
        *v = !(*v);
        state.targetState = *v;
        state.isAnimating = true;
        state.lastUpdate = now;
        ImGui::MarkItemEdited(id);
    }

    if (state.isAnimating) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.lastUpdate).count();
        float animation_progress = std::min(1.0f, (float)elapsed / 200.0f);

        if (animation_progress >= 1.0f) {
            state.knobPosition = state.targetState ? 1.0f : 0.0f;
            state.isAnimating = false;
        } else {
            float eased = 1.0f - std::pow(1.0f - animation_progress, 3);
            float target = state.targetState ? 1.0f : 0.0f;
            state.knobPosition = target * eased + (state.targetState ? 0 : 1) * (1.0f - eased);
        }
    }

    ImDrawList* draw_list = w->DrawList;

    float switch_x = bb.Max.x - toggle_width - 10.0f;
    ImVec2 toggle_bg_min(switch_x, pos.y + (toggle_height * 0.1f));
    ImVec2 toggle_bg_max(switch_x + toggle_width, pos.y + toggle_height * 1.1f);

    float t = state.knobPosition;

    ImVec4 off_bg = g_darkMode ? ImVec4(0.24f, 0.24f, 0.28f, 1.00f) : ImVec4(0.55f, 0.55f, 0.58f, 1.00f);
    ImVec4 on_bg = ImVec4(g_ThemeColorArr[0], g_ThemeColorArr[1], g_ThemeColorArr[2], 1.00f);

    ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(
        off_bg.x + (on_bg.x - off_bg.x) * t,
        off_bg.y + (on_bg.y - off_bg.y) * t,
        off_bg.z + (on_bg.z - off_bg.z) * t,
        1.00f
    ));

    float rounding = toggle_height * 0.5f;

    if (hovered && g_GlowIntensity > 0.1f) {
        for (int i = 1; i <= 2; i++) {
            float glow_alpha = (g_GlowIntensity * 0.12f) / i;
            draw_list->AddRectFilled(
                ImVec2(toggle_bg_min.x - i, toggle_bg_min.y - i),
                ImVec2(toggle_bg_max.x + i, toggle_bg_max.y + i),
                ImGui::ColorConvertFloat4ToU32(ImVec4(g_ThemeColorArr[0], g_ThemeColorArr[1], g_ThemeColorArr[2], glow_alpha)),
                rounding + i
            );
        }
    }

    draw_list->AddRectFilled(toggle_bg_min, toggle_bg_max, bg_color, rounding);

    if (t < 0.99f) {
        ImU32 border_color = g_darkMode ? IM_COL32(60, 60, 65, (int)((1.0f - t) * 255)) : IM_COL32(100, 100, 105, (int)((1.0f - t) * 255));
        draw_list->AddRect(toggle_bg_min, toggle_bg_max, border_color, rounding, 0, 1.2f);
    }

    float padding_offset = 2.0f;
    float knob_x = toggle_bg_min.x + padding_offset + (toggle_width - knob_size - (padding_offset * 2.0f)) * t;
    float knob_y = toggle_bg_min.y + (toggle_bg_max.y - toggle_bg_min.y - knob_size) * 0.5f;

    draw_list->AddCircleFilled(
        ImVec2(knob_x + knob_size * 0.5f + 1.0f, knob_y + knob_size * 0.5f + 1.0f),
        knob_size * 0.5f,
        IM_COL32(0, 0, 0, 45)
    );

    draw_list->AddCircleFilled(
        ImVec2(knob_x + knob_size * 0.5f, knob_y + knob_size * 0.5f),
        knob_size * 0.5f,
        g_darkMode ? IM_COL32(230, 230, 235, 255) : IM_COL32(255, 255, 255, 255)
    );

    ImVec2 text_pos = ImVec2(pos.x, pos.y + (toggle_height - label_size.y) * 0.5f);

    ImU32 text_color_to_use;
    if (g_darkMode) {
        text_color_to_use = hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 200, 205, 255);
    } else {
        text_color_to_use = hovered ? IM_COL32(20, 20, 20, 255) : IM_COL32(50, 50, 50, 255);
    }

    draw_list->AddText(text_pos, text_color_to_use, label);

    return pressed;
}

bool DrawColorPickerRow(const char* label, float* color, const char* pickerID) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImVec2 pos = window->DC.CursorPos;
    float width = ImGui::GetContentRegionAvail().x;
    float height = ImGui::GetFrameHeight() * 1.35f;

    const ImRect total_bb(pos, ImVec2(pos.x + width, pos.y + height));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, window->GetID(pickerID))) return false;

    ImGui::RenderText(ImVec2(pos.x + 5.0f, pos.y + (height - ImGui::CalcTextSize(label).y) * 0.5f), label);

    float rightAlignX = pos.x + width - 55.0f;
    ImGui::SetCursorScreenPos(ImVec2(rightAlignX, pos.y + (height - ImGui::GetFrameHeight() * 0.75f) * 0.5f));

    bool changed = ImGui::ColorEdit4(pickerID, color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + height + style.ItemSpacing.y));
    return changed;
}

void ApplyCustomTheme(float scale_y = 1.0f) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    style.WindowRounding = 12.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 6.0f;

    float calculatedGap = 20.0f * scale_y;
    style.ItemSpacing = ImVec2(12.0f, std::max(1.0f, std::min(calculatedGap, 26.0f)));

    float paddingY = 10.0f * scale_y;
    style.FramePadding = ImVec2(10.0f, std::max(4.0f, paddingY));

    if (!g_darkMode) {
        colors[ImGuiCol_WindowBg]             = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_Border]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Text]                 = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_Button]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
        colors[ImGuiCol_FrameBg]              = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.89f, 0.89f, 0.89f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    } else {
        colors[ImGuiCol_WindowBg]             = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_Border]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Text]                 = ImVec4(0.95f, 0.95f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
        colors[ImGuiCol_Button]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_FrameBg]              = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.18f, 0.18f, 0.24f, 1.00f);
    }

    colors[ImGuiCol_SliderGrab]           = g_ThemeColor;
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(g_ThemeColorArr[0] * 0.9f, g_ThemeColorArr[1] * 0.9f, g_ThemeColorArr[2] * 0.9f, g_ThemeColorArr[3]);
    colors[ImGuiCol_Header]               = g_ThemeColor;
    colors[ImGuiCol_HeaderHovered]        = g_ThemeColor;
    colors[ImGuiCol_HeaderActive]         = g_ThemeColor;
}

void RenderMenu() {
    g_MenuWidth = g_MenuSize;
    g_MenuHeight = g_MenuSize * 0.5862f;

    float scale_y = g_MenuHeight / 850.0f;
    scale_y = std::max(0.45f, std::min(scale_y, 1.2f));

    ApplyCustomTheme(scale_y);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = (scale_y < 0.8f) ? 1.05f : 1.30f;

    ImGui::SetNextWindowSize(ImVec2(g_MenuWidth, g_MenuHeight), ImGuiCond_Always);
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("STRANGER CHEATS", nullptr, windowFlags);

    float sidebarWidth = 280.0f;

    ImGui::BeginChild("##LeftSidebar", ImVec2(sidebarWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetCursorPosX(8);
        ImGui::SetCursorPosY(35 * scale_y);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f));
        ImGui::TextColored(g_ThemeColor, "@XANOUAR");
        ImGui::SameLine();
        ImGui::TextColored(g_darkMode ? ImVec4(0.95f, 0.95f, 0.98f, 1.00f) : ImVec4(0.24f, 0.24f, 0.24f, 1.00f), "CHEATER");
        ImGui::PopStyleVar();

        ImGui::SetCursorPosY(80 * scale_y);
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(ImGui::GetCursorScreenPos().x + 20, ImGui::GetCursorScreenPos().y),
            ImVec2(ImGui::GetCursorScreenPos().x + sidebarWidth - 40, ImGui::GetCursorScreenPos().y),
            g_darkMode ? IM_COL32(60, 60, 65, 255) : IM_COL32(220, 220, 220, 255),
            1.5f
        );

        ImGui::SetCursorPosY(105 * scale_y);

        const char* tabNames[6] = { "Aimbot", "Visuals", "Config", "Misc", "Settings", "Touch Aimbot" };

        float baseAvailableHeight = g_MenuHeight - (145.0f * scale_y);
        float tabHeight = baseAvailableHeight / 6.0f;
        tabHeight = std::max(22.0f, std::min(tabHeight, 75.0f));

        for (int i = 0; i < 6; i++) {
            bool isSelected = (currentTab == i);

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, g_ThemeColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, g_ThemeColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, g_ThemeColor);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, g_darkMode ? ImVec4(0.14f, 0.14f, 0.18f, 1.00f) : ImVec4(0.92f, 0.92f, 0.92f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, g_darkMode ? ImVec4(0.18f, 0.18f, 0.22f, 1.00f) : ImVec4(0.87f, 0.87f, 0.87f, 1.00f));
            }

            ImGui::SetCursorPosX(15);

            char hiddenLabel[64];
            sprintf(hiddenLabel, "##TabButton_%d", i);

            if (ImGui::Button(hiddenLabel, ImVec2(sidebarWidth - 30.0f, tabHeight - 3.0f))) {
                currentTab = i;
            }

            ImVec2 b_pos = ImGui::GetItemRectMin();
            float b_height = ImGui::GetItemRectSize().y;
            ImVec2 cp(b_pos.x + 28.0f, b_pos.y + b_height / 2.0f);

            ImU32 drawColor = isSelected ? IM_COL32(255, 255, 255, 255) : (g_darkMode ? IM_COL32(180, 180, 185, 255) : IM_COL32(110, 110, 110, 255));
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            if (i == 2) {
                float s = scale_y * 1.15f;
                ImU32 outlineColor = drawColor;
                ImU32 fillColor = (drawColor & 0x00FFFFFF) | (35 << 24);

                draw_list->AddCircleFilled(ImVec2(cp.x - 3.0f * s, cp.y + 1.0f * s), 10.5f * s, fillColor);
                draw_list->AddCircleFilled(ImVec2(cp.x + 4.0f * s, cp.y - 1.0f * s), 8.0f * s, fillColor);

                draw_list->PathClear();
                draw_list->PathArcTo(ImVec2(cp.x - 3.0f * s, cp.y + 1.0f * s), 10.5f * s, IM_PI * 0.4f, IM_PI * 1.6f, 16);
                draw_list->PathArcTo(ImVec2(cp.x + 4.0f * s, cp.y - 1.0f * s), 8.0f * s, -IM_PI * 0.6f, IM_PI * 0.5f, 16);
                draw_list->PathStroke(outlineColor, true, 1.8f);

                draw_list->AddCircle(ImVec2(cp.x - 4.5f * s, cp.y + 3.0f * s), 2.2f * s, outlineColor, 12, 1.5f);
                draw_list->AddCircleFilled(ImVec2(cp.x + 2.5f * s, cp.y - 4.0f * s), 1.8f * s, outlineColor);
                draw_list->AddCircleFilled(ImVec2(cp.x + 5.5f * s, cp.y + 2.0f * s), 1.8f * s, outlineColor);
                draw_list->AddCircleFilled(ImVec2(cp.x - 1.0f * s, cp.y - 4.5f * s), 1.8f * s, outlineColor);

                draw_list->AddLine(ImVec2(cp.x - 14.0f * s, cp.y + 12.0f * s), ImVec2(cp.x + 4.0f * s, cp.y - 6.0f * s), outlineColor, 1.8f * s);
                draw_list->AddLine(ImVec2(cp.x + 4.0f * s, cp.y - 6.0f * s), ImVec2(cp.x + 7.0f * s, cp.y - 9.0f * s), outlineColor, 2.5f * s);

                draw_list->AddTriangleFilled(
                    ImVec2(cp.x + 7.0f * s, cp.y - 9.0f * s),
                    ImVec2(cp.x + 12.0f * s, cp.y - 14.0f * s),
                    ImVec2(cp.x + 12.5f * s, cp.y - 8.0f * s),
                    outlineColor
                );
            }
            else if (i == 3) {
                ImVec2 t_top(cp.x, cp.y - 15.0f * scale_y);
                ImVec2 t_left(cp.x - 15.0f * scale_y, cp.y - 7.0f * scale_y);
                ImVec2 t_right(cp.x + 15.0f * scale_y, cp.y - 7.0f * scale_y);
                ImVec2 t_bottom(cp.x, cp.y + 1.0f * scale_y);
                ImVec2 b_left(cp.x - 15.0f * scale_y, cp.y + 8.0f * scale_y);
                ImVec2 b_right(cp.x + 15.0f * scale_y, cp.y + 8.0f * scale_y);
                ImVec2 b_bottom(cp.x, cp.y + 16.0f * scale_y);
                draw_list->AddLine(t_top, t_left, drawColor, 1.8f);
                draw_list->AddLine(t_left, t_bottom, drawColor, 1.8f);
                draw_list->AddLine(t_bottom, t_right, drawColor, 1.8f);
                draw_list->AddLine(t_right, t_top, drawColor, 1.8f);
                draw_list->AddLine(t_left, b_left, drawColor, 1.8f);
                draw_list->AddLine(t_bottom, b_bottom, drawColor, 2.5f);
                draw_list->AddLine(t_right, b_right, drawColor, 1.8f);
                draw_list->AddLine(b_left, b_bottom, drawColor, 1.8f);
                draw_list->AddLine(b_bottom, b_right, drawColor, 1.8f);
            }
            else if (i == 5) {
                draw_list->AddCircle(cp, 13.0f * scale_y, drawColor, 0, 2.2f);
                draw_list->AddCircleFilled(ImVec2(cp.x, cp.y - 4.5f * scale_y), 2.0f * scale_y, drawColor);
                draw_list->AddLine(ImVec2(cp.x, cp.y - 1.0f * scale_y), ImVec2(cp.x, cp.y + 6.0f * scale_y), drawColor, 2.5f);
                draw_list->AddLine(ImVec2(cp.x - 2.5f * scale_y, cp.y + 6.0f * scale_y), ImVec2(cp.x + 2.5f * scale_y, cp.y + 6.0f * scale_y), drawColor, 1.5f);
            }
            else if (g_IconFont) {
                const char* iconChar = "";
                switch (i) {
                    case 0: iconChar = "K"; break;
                    case 1: iconChar = "A"; break;
                    case 4: iconChar = "M"; break;
                    default: iconChar = "A"; break;
                }

                float iconSize = 32.0f * scale_y;
                ImGui::PushFont(g_IconFont);

                ImVec2 iconTextSize = g_IconFont->CalcTextSizeA(iconSize, FLT_MAX, 0.0f, iconChar);
                ImVec2 iconPos(cp.x - (iconTextSize.x * 0.5f), cp.y - (iconTextSize.y * 0.5f));

                if (isSelected && g_GlowIntensity > 0.1f) {
                    draw_list->AddText(g_IconFont, iconSize, ImVec2(iconPos.x + 1.0f, iconPos.y + 1.0f),
                        ImGui::ColorConvertFloat4ToU32(ImVec4(g_ThemeColorArr[0], g_ThemeColorArr[1], g_ThemeColorArr[2], g_GlowIntensity * 0.45f)), iconChar);
                }

                draw_list->AddText(g_IconFont, iconSize, iconPos, drawColor, iconChar);
                ImGui::PopFont();
            }
            else {
                ImU32 themeColorU32 = ImGui::ColorConvertFloat4ToU32(g_ThemeColor);
                draw_list->AddCircle(cp, 10.0f * scale_y, drawColor, 0, 2.0f);
                draw_list->AddCircleFilled(cp, 4.0f * scale_y, themeColorU32);
            }

            ImVec2 textPos(b_pos.x + 60.0f, b_pos.y + (b_height - ImGui::CalcTextSize(tabNames[i]).y) * 0.5f);
            draw_list->AddText(textPos, drawColor, tabNames[i]);

            ImGui::PopStyleColor(3);
            ImGui::Spacing();
        }
    }
    ImGui::EndChild();

    ImVec2 dividerStart = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(dividerStart.x, dividerStart.y + 20.0f * scale_y),
        ImVec2(dividerStart.x, dividerStart.y + ImGui::GetContentRegionAvail().y - 20.0f * scale_y),
        g_darkMode ? IM_COL32(60, 60, 65, 255) : IM_COL32(225, 225, 225, 255),
        1.5f
    );

    ImGui::SameLine(0, 30.0f);

    ImGui::BeginChild("##MainDynamicPanel", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetCursorPosY(25 * scale_y);

        if (currentTab == 1)
        {
            float availableContentWidth = ImGui::GetContentRegionAvail().x;
            float leftColumnWidth = (availableContentWidth - 30.0f) * 0.50f;
            float rightColumnWidth = (availableContentWidth - 30.0f) * 0.50f;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 38.0f * scale_y));

            ImGui::BeginChild("##BasicEspPanel", ImVec2(leftColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 4.0f));
                ImGui::TextColored(g_ThemeColor, "BASIC ESP");
                ImGui::Separator();
                ImGui::PopStyleVar();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f * scale_y);

                ModernToggle("Line ESP", &showLine);
                ModernToggle("Box ESP", &showBox);
                ModernToggle("Health Bar", &showHealth);
                ModernToggle("Player Counter", &showCount);
                ModernToggle("Name ESP", &showName);
                ModernToggle("Distance ESP", &showDistance);
                ModernToggle("Show Health Text", &showHealthText);
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 10.0f);

            ImVec2 divPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(divPos.x, divPos.y + 10.0f * scale_y),
                ImVec2(divPos.x, divPos.y + ImGui::GetContentRegionAvail().y - 30.0f * scale_y),
                g_darkMode ? IM_COL32(60, 60, 65, 255) : IM_COL32(220, 220, 220, 255),
                1.0f
            );

            ImGui::SameLine(0, 20.0f);

            ImGui::BeginChild("##AdvancedEspPanel", ImVec2(rightColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 4.0f));
                ImGui::TextColored(g_ThemeColor, "ADVANCED ESP");
                ImGui::Separator();
                ImGui::PopStyleVar();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f * scale_y);

                ModernToggle("Skeleton ESP", &showSkeleton);
                ModernToggle("360 Alert", &is3600);
            }
            ImGui::EndChild();

            ImGui::PopStyleVar();
        }
        else if (currentTab == 0)
        {
            float availableContentWidth = ImGui::GetContentRegionAvail().x;
            float leftColumnWidth = (availableContentWidth - 30.0f) * 0.50f;
            float rightColumnWidth = (availableContentWidth - 30.0f) * 0.50f;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 38.0f * scale_y));

            ImGui::BeginChild("##AimbotMainPanel", ImVec2(leftColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 4.0f));
                ImGui::TextColored(g_ThemeColor, "AIM ASSIST");
                ImGui::Separator();
                ImGui::PopStyleVar();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f * scale_y);

                ModernToggle("Aim bot", &Aimbot);
                ModernToggle("Aim Collider", &AimLock);
                ModernToggle("Aim Visible", &AimVisible);
                ModernToggle("Aim Silent(Risk)", &AimSilent);
                if (AimSilent) {
                    ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);
                    ImGui::SliderInt("##silent_aim_pct", &SilentAimPercentage, 0, 100, "Silent Aim: %d%%");
                }

                ImGui::Spacing();
                ImGui::Text("Aimbot Bone Target");
                ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);

                const char* localizedBones[] = { "Head", "Body" };
                ImGui::Combo("##bone_target_combo", &selectedBoneIndex, localizedBones, IM_ARRAYSIZE(localizedBones));
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 10.0f);

            ImVec2 divPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(divPos.x, divPos.y + 10.0f * scale_y),
                ImVec2(divPos.x, divPos.y + ImGui::GetContentRegionAvail().y - 30.0f * scale_y),
                g_darkMode ? IM_COL32(60, 60, 65, 255) : IM_COL32(220, 220, 220, 255),
                1.0f
            );

            ImGui::SameLine(0, 20.0f);

            ImGui::BeginChild("##AimbotConfigPanel", ImVec2(rightColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 4.0f));
                ImGui::TextColored(g_ThemeColor, "AIM SETTINGS");
                ImGui::Separator();
                ImGui::PopStyleVar();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f * scale_y);

                ModernToggle("Ignore Knocked", &Ignorknock);
                ModernToggle("Aim Line", &showAimLine);
                ModernToggle("Show Aim FOV", &showAimFov);

                ImGui::Spacing();
                ImGui::Text("Aim FOV Range Size");
                ImGui::SetNextItemWidth(rightColumnWidth * 0.85f);
                ImGui::SliderFloat("##fov_range_slider", &AimFov, 0.0f, 360.0f, "%.1f px");
            }
            ImGui::EndChild();

            ImGui::PopStyleVar();
        }
        else if (currentTab == 2)
        {
            float availableContentWidth = ImGui::GetContentRegionAvail().x;
            float leftColumnWidth = (availableContentWidth - 30.0f) * 0.50f;
            float rightColumnWidth = (availableContentWidth - 30.0f) * 0.50f;

            ImGui::BeginChild("##ConfigLeftPanel", ImVec2(leftColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::SetCursorPosY(10.0f * scale_y);

                ImGui::Text("Box Style Mode");
                ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);

                const char* styles[] = {
                    "Default Flat", "Solid Filled", "Border Outline", "Corners", "Outer Glow",
                    "Dashed Lines", "Double Boxes", "Rounded Corners", "3D Filled Block"
                };
                ImGui::Combo("##box_style_combo", &boxStyle, styles, IM_ARRAYSIZE(styles));

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f * scale_y);
                ImGui::Text("Box Color Preset Mode");
                ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);

                const char* colorModes[] = {
                    "Custom Choice", "Health Gradient", "Distance Scaling", "Rainbow Hue", "Cyan Preset"
                };
                ImGui::Combo("##box_color_mode_combo", &boxColorMode, colorModes, IM_ARRAYSIZE(colorModes));

                if (boxColorMode == 0) {
                    DrawColorPickerRow("Custom Box Color", (float*)&boxCustomColor, "##custom_box_color_picker");
                }
                if (lineColorMode == 0) {
                    DrawColorPickerRow("Custom Line Color", (float*)&lineCustomColor, "##custom_line_color_picker");
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f * scale_y);
                ImGui::Text("Box Line Thickness");
                ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);
                ImGui::SliderFloat("##box_thick_slider", &boxThickness, 1.0f, 5.0f, "%.1f px");

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f * scale_y);
                ImGui::Text("ESP Text Label Size");
                ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);
                ImGui::SliderInt("##text_size_slider", &textSize, 10, 30, "%d pt");
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 10.0f);

            ImVec2 divPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(divPos.x, divPos.y + 10.0f * scale_y),
                ImVec2(divPos.x, divPos.y + ImGui::GetContentRegionAvail().y - 30.0f * scale_y),
                g_darkMode ? IM_COL32(60, 60, 65, 255) : IM_COL32(220, 220, 220, 255),
                1.0f
            );

            ImGui::SameLine(0, 20.0f);

            ImGui::BeginChild("##ConfigRightPanel", ImVec2(rightColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::SetCursorPosY(10.0f * scale_y);

                ImGui::Text("Line Style Mode");
                ImGui::SetNextItemWidth(rightColumnWidth * 0.85f);

                const char* lineStyles[] = {
                    "Solid Line", "Dot Trail", "Dashed Track", "Double Line", "Rainbow Line",
                    "Glow Effect", "Arrow Tracker", "Thin Double", "Dual Highlight", "Standard Flat"
                };
                ImGui::Combo("##line_style_combo", &lineStyle, lineStyles, IM_ARRAYSIZE(lineStyles));

                ImGui::Spacing();
                ImGui::Text("Line Color Preset Mode");
                ImGui::SetNextItemWidth(rightColumnWidth * 0.85f);

                const char* lineColorModes[] = {
                    "Custom Choice", "Health Gradient", "Distance Scaling", "Rainbow Hue", "Cyan Preset"
                };
                ImGui::Combo("##line_color_mode_combo", &lineColorMode, lineColorModes, IM_ARRAYSIZE(lineColorModes));

                ImGui::Spacing();
                ImGui::Text("Line Thickness");
                ImGui::SetNextItemWidth(rightColumnWidth * 0.85f);
                ImGui::SliderFloat("##line_thick_slider", &lineThickness, 1.0f, 5.0f, "%.1f px");

                ImGui::Spacing();
                ImGui::Text("Line Start Position");
                ImGui::SetNextItemWidth(rightColumnWidth * 0.85f);

                const char* startModes[3];
                startModes[0] = "Top";
                startModes[1] = "Center";
                startModes[2] = "Bottom";
                ImGui::Combo("##start_modes_combo", &lineStartPosition, startModes, IM_ARRAYSIZE(startModes));
            }
            ImGui::EndChild();
        }
        else if (currentTab == 3)
        {
            float availableContentWidth = ImGui::GetContentRegionAvail().x;
            float leftColumnWidth = (availableContentWidth - 20.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 32.0f * scale_y));

            ImGui::BeginChild("##MiscLeftPanel", ImVec2(leftColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 4.0f));
                ImGui::TextColored(g_ThemeColor, "ADVANCE FEATURE");
                ImGui::Separator();
                ImGui::PopStyleVar();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f * scale_y);

                ModernToggle("Speed Hack(Risk)", &SpeedHackV2);

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 6.0f * scale_y));

                ImGui::Text("Speed Multiplier");
                ImGui::SetNextItemWidth(leftColumnWidth * 0.85f);
                ImGui::SliderFloat("##speed_multiplier_slider", &SpeedMultiplier, 1.0f, 5.0f, "%.2f x");

                ImGui::PopStyleVar();

                ModernToggle("Ghost Hack", &GhostHack);
                ModernToggle("No Reload", &NoReload);
                ModernToggle("No Recoil", &NoRecoil);

                
            }
            ImGui::EndChild();

            ImGui::PopStyleVar();
        }
        else if (currentTab == 4)
        {
            float availableContentWidth = ImGui::GetContentRegionAvail().x;

            ImGui::BeginChild("##SettingsSingleUnifiedPanel", ImVec2(availableContentWidth - 10.0f, 0), false, ImGuiWindowFlags_NoScrollbar);
            {
                ImGuiWindow* w = ImGui::GetCurrentWindow();
                ImDrawList* dl = w->DrawList;

                ImGui::SetCursorPosY(5.0f * scale_y);

                ImVec2 p = ImGui::GetCursorScreenPos();
                float ww = availableContentWidth - 25.0f;

                float pad = 16.0f;
                float box_h = std::max(36.0f, 60.0f * scale_y);
                float gap = std::max(4.0f, 12.0f * scale_y);
                float fontSize = ImGui::GetFontSize();
                float frameHeight = ImGui::GetFrameHeight();

                ImU32 panel_color = g_darkMode ? IM_COL32(20, 20, 25, 255) : IM_COL32(242, 242, 245, 255);
                ImU32 border_color = g_darkMode ? IM_COL32(40, 40, 45, 255) : IM_COL32(220, 220, 225, 255);
                ImU32 text_dim_color = g_darkMode ? IM_COL32(170, 170, 180, 255) : IM_COL32(110, 110, 115, 255);

                ImVec2 boxWidth_p(p.x, p.y);
                dl->AddRectFilled(boxWidth_p, ImVec2(boxWidth_p.x + ww, boxWidth_p.y + box_h), panel_color, 8.0f);
                dl->AddRect(boxWidth_p, ImVec2(boxWidth_p.x + ww, boxWidth_p.y + box_h), border_color, 8.0f, 0, 1.0f);
                dl->AddText(ImVec2(boxWidth_p.x + pad, boxWidth_p.y + (box_h - fontSize) * 0.5f), text_dim_color, "Menu Overall Size");

                ImGui::SetCursorScreenPos(ImVec2(boxWidth_p.x + ww - pad - 260.0f - 20.0f, boxWidth_p.y + (box_h - frameHeight) * 0.5f));
                ImGui::SetNextItemWidth(260.0f);

                const char* menuSizes[] = { "800 px", "1200 px", "1400 px", "1600 px", "1800 px", "2000 px" };
                static int currentSizeIndex = 2;

                if (ImGui::Combo("##menu_size_combo", &currentSizeIndex, menuSizes, IM_ARRAYSIZE(menuSizes))) {
                    if (currentSizeIndex == 0) g_MenuSize = 800.0f;
                    else if (currentSizeIndex == 1) g_MenuSize = 1200.0f;
                    else if (currentSizeIndex == 2) g_MenuSize = 1400.0f;
                    else if (currentSizeIndex == 3) g_MenuSize = 1600.0f;
                    else if (currentSizeIndex == 4) g_MenuSize = 1800.0f;
                    else if (currentSizeIndex == 5) g_MenuSize = 2000.0f;
                }
                ImGui::Dummy(ImVec2(0, box_h + gap));

                ImVec2 boxApp_p(p.x, p.y + (box_h + gap) * 1);
                dl->AddRectFilled(boxApp_p, ImVec2(boxApp_p.x + ww, boxApp_p.y + box_h), panel_color, 8.0f);
                dl->AddRect(boxApp_p, ImVec2(boxApp_p.x + ww, boxApp_p.y + box_h), border_color, 8.0f, 0, 1.0f);
                dl->AddText(ImVec2(boxApp_p.x + pad, boxApp_p.y + (box_h - fontSize) * 0.5f), text_dim_color, "Appearance Theme Profile");

                float btnWidth = 160.0f;
                float btnHeight = 36.0f;
                ImVec2 btnPos(boxApp_p.x + ww - pad - btnWidth - 75.0f, boxApp_p.y + (box_h - btnHeight) * 0.5f);

                ImGui::SetCursorScreenPos(btnPos);
                if (ImGui::InvisibleButton("##AppearanceToggleButton", ImVec2(btnWidth, btnHeight))) {
                    g_darkMode = !g_darkMode;
                }

                bool appHovered = ImGui::IsItemHovered();
                ImU32 btnBg = appHovered ? (g_darkMode ? IM_COL32(45, 45, 55, 255) : IM_COL32(225, 225, 230, 255))
                                         : (g_darkMode ? IM_COL32(35, 35, 42, 255) : IM_COL32(235, 235, 240, 255));
                dl->AddRectFilled(btnPos, ImVec2(btnPos.x + btnWidth, btnPos.y + btnHeight), btnBg, 6.0f);
                dl->AddRect(btnPos, ImVec2(btnPos.x + btnWidth, btnPos.y + btnHeight), border_color, 6.0f, 0, 1.0f);

                ImVec2 iconCenter(btnPos.x + 12.0f, btnPos.y + btnHeight * 0.5f);
                if (!g_darkMode) {
                    dl->AddCircleFilled(iconCenter, 5.0f, IM_COL32(251, 191, 36, 255));
                    for (int r = 0; r < 8; r++) {
                        float angle = r * (IM_PI / 4.0f);
                        ImVec2 start(iconCenter.x + cosf(angle) * 7.0f, iconCenter.y + sinf(angle) * 7.0f);
                        ImVec2 end(iconCenter.x + cosf(angle) * 10.0f, iconCenter.y + sinf(angle) * 10.0f);
                        dl->AddLine(start, end, IM_COL32(251, 191, 36, 255), 1.2f);
                    }
                    dl->AddText(ImGui::GetFont(), fontSize * 1.15f, ImVec2(btnPos.x + 20.0f, btnPos.y + (btnHeight - (fontSize * 1.15f)) * 0.5f), g_darkMode ? IM_COL32(255, 255, 255, 255) : IM_COL32(30, 30, 30, 255), "Light Mode");
                } else {
                    dl->PathClear();
                    dl->PathArcTo(iconCenter, 7.0f, -IM_PI * 0.45f, IM_PI * 0.85f, 32);
                    dl->PathArcTo(ImVec2(iconCenter.x - 3.0f, iconCenter.y - 1.0f), 5.5f, IM_PI * 0.75f, -IM_PI * 0.35f, 32);
                    dl->PathFillConvex(IM_COL32(147, 197, 253, 255));
                    dl->AddText(ImGui::GetFont(), fontSize * 1.15f, ImVec2(btnPos.x + 20.0f, btnPos.y + (btnHeight - (fontSize * 1.15f)) * 0.5f), g_darkMode ? IM_COL32(255, 255, 255, 255) : IM_COL32(30, 30, 30, 255), "Dark Mode");
                }
                ImGui::Dummy(ImVec2(0, box_h + gap));

                ImVec2 boxTheme_p(p.x, p.y + (box_h + gap) * 2);
                float boxTheme_h = std::max(120.0f, 175.0f * scale_y);
                dl->AddRectFilled(boxTheme_p, ImVec2(boxTheme_p.x + ww, boxTheme_p.y + boxTheme_h), panel_color, 8.0f);
                dl->AddRect(boxTheme_p, ImVec2(boxTheme_p.x + ww, boxTheme_p.y + boxTheme_h), border_color, 8.0f, 0, 1.0f);

                float swatchW = std::max(55.0f, 80.0f * scale_y);
                float swatchH = std::max(28.0f, 40.0f * scale_y);
                float swatchG = std::max(6.0f, 12.0f * scale_y);
                float startX = boxTheme_p.x + pad;
                float startY = boxTheme_p.y + (50.0f * scale_y);

                struct Swatch {
                    ImVec4 col;
                    bool recommended;
                };

                Swatch swatches[6] = {
                    { ImVec4(1.00f, 0.00f, 0.00f, 1.00f), false },
                    { ImVec4(0.00f, 1.00f, 0.00f, 1.00f), false },
                    { ImVec4(0.00f, 0.00f, 1.00f, 1.00f), false },
                    { ImVec4(1.00f, 0.00f, 1.00f, 1.00f), false },
                    { ImVec4(1.00f, 1.00f, 0.00f, 1.00f), false },
                    { ImVec4(0.00f, 1.00f, 1.00f, 1.00f), true }
                };

                for (int s = 0; s < 6; s++) {
                    ImGui::PushID(s);
                    ImVec2 swatchPos(startX + s * (swatchW + swatchG), startY);

                    ImGui::SetCursorScreenPos(swatchPos);
                    if (ImGui::InvisibleButton("##color_swatch_button", ImVec2(swatchW, swatchH))) {
                        g_ThemeColorArr[0] = swatches[s].col.x;
                        g_ThemeColorArr[1] = swatches[s].col.y;
                        g_ThemeColorArr[2] = swatches[s].col.z;
                        g_ThemeColorArr[3] = swatches[s].col.w;
                    }

                    bool swatchHovered = ImGui::IsItemHovered();
                    ImDrawList* swatch_dl = ImGui::GetWindowDrawList();

                    swatch_dl->AddRectFilled(swatchPos, ImVec2(swatchPos.x + swatchW, swatchPos.y + swatchH), ImGui::ColorConvertFloat4ToU32(swatches[s].col), 8.0f);

                    if (g_ThemeColorArr[0] == swatches[s].col.x && g_ThemeColorArr[1] == swatches[s].col.y && g_ThemeColorArr[2] == swatches[s].col.z) {
                        swatch_dl->AddRect(swatchPos, ImVec2(swatchPos.x + swatchW, swatchPos.y + swatchH), g_darkMode ? IM_COL32(255, 255, 255, 255) : IM_COL32(20, 20, 20, 255), 8.0f, 0, 2.5f);
                    } else if (swatchHovered) {
                        swatch_dl->AddRect(swatchPos, ImVec2(swatchPos.x + swatchW, swatchPos.y + swatchH), IM_COL32(180, 180, 180, 200), 8.0f, 0, 1.5f);
                    }

                    if (swatches[s].recommended) {
                        const char* recText = "RECOMMENDED";
                        float recFontSize = fontSize * 0.85f * scale_y;
                        float textWidth = ImGui::GetFont()->CalcTextSizeA(recFontSize, FLT_MAX, 0.0f, recText).x;
                        ImVec2 recTextPos(swatchPos.x + (swatchW - textWidth) * 0.5f - 18.0f, swatchPos.y - (36.0f * scale_y));
                        swatch_dl->AddText(ImGui::GetFont(), recFontSize, recTextPos, ImGui::ColorConvertFloat4ToU32(swatches[s].col), recText);
                    }

                    ImGui::PopID();
                }

                float pickerY = startY + swatchH + (18.0f * scale_y);
                dl->AddText(ImVec2(boxTheme_p.x + pad, pickerY + (35.0f * scale_y - fontSize) * 0.5f), text_dim_color, "Custom Menu Color");

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
                ImGui::SetCursorScreenPos(ImVec2(boxTheme_p.x + ww - pad - 40.0f, pickerY + (35.0f * scale_y - (fontSize + 6.0f)) * 0.5f));
                ImGui::ColorEdit3("##global_theme_color_picker", g_ThemeColorArr, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::PopStyleVar();

                ImGui::Dummy(ImVec2(0, boxTheme_h + gap));

                ImGui::Dummy(ImVec2(0, std::max(15.0f, 35.0f * scale_y)));

                ImGui::PushStyleColor(ImGuiCol_Button, g_ThemeColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(g_ThemeColorArr[0] * 0.9f, g_ThemeColorArr[1] * 0.9f, g_ThemeColorArr[2] * 0.9f, g_ThemeColorArr[3]));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(g_ThemeColorArr[0] * 0.8f, g_ThemeColorArr[1] * 0.8f, g_ThemeColorArr[2] * 0.8f, g_ThemeColorArr[3]));

                if (ImGui::Button("EXIT AND SHUTDOWN CHEAT", ImVec2(ww, std::max(40.0f, 65.0f * scale_y)))) {
                    _exit(0);
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::EndChild();
        }
        else if (currentTab == 5)
{
    
    }
    ImGui::EndChild();

    ImGui::PopStyleVar();
}
    
    ImGui::EndChild();

    ImGui::End();
}

void LoadCustomIconFonts(ImGuiIO& io) {
    io.Fonts->AddFontFromMemoryTTF(inter_medium, sizeof(inter_medium), 20.f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    g_BoldFont = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 23.f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    g_IconFont = io.Fonts->AddFontFromMemoryTTF((void *)F107_data, F107_size, 25.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    io.Fonts->Build();
}

bool isProcessAlive(int pid) {
    if (pid <= 0) return false;
    char path[64];
    sprintf(path, oxorany("/proc/%d"), pid);
    struct stat st;
    return (stat(path, &st) == 0);
}

void VolumeKeyListenerThread() {
    int fds[16];
    int fd_count = 0;

    for (int i = 0; i < 16; i++) {
        char path[64];
        sprintf(path, oxorany("/dev/input/event%d"), i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) fds[fd_count++] = fd;
    }

    if (fd_count == 0) {
        while (main_thread_flag) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        return;
    }

    struct input_event ev;
    while (main_thread_flag) {
        for (int i = 0; i < fd_count; i++) {
            while (read(fds[i], &ev, sizeof(struct input_event)) > 0) {
                if (ev.type == EV_KEY) {
                    if ((ev.code == KEY_VOLUMEUP || ev.code == KEY_VOLUMEDOWN) && ev.value == 1) {
                        showMenu = !showMenu;
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    for (int i = 0; i < fd_count; i++) close(fds[i]);
}

int main(int argc, char *argv[]) {
    game_pid = getProcessID(packageName);
    while (game_pid <= 0) {
        game_pid = getProcessID(packageName);
        sleep(1);
    }

    Memory::g_pid = (pid_t)game_pid;
    driver->initialize((pid_t)game_pid);

    lib_base  = get_module_base(targetaLibName,  PERM_EXEC);
    lib_base2 = get_module_base(targetaLibName2, PERM_EXEC);

    screen_config();

    ::abs_ScreenX = (displayInfo.height > displayInfo.width ? displayInfo.height : displayInfo.width);
    ::abs_ScreenY = (displayInfo.height < displayInfo.width ? displayInfo.height : displayInfo.width);

    ::native_window_screen_x = (displayInfo.height > displayInfo.width ? displayInfo.height : displayInfo.width);
    ::native_window_screen_y = (displayInfo.height < displayInfo.width ? displayInfo.height : displayInfo.width);

    if (!initGUI_draw(native_window_screen_x, native_window_screen_y, true)) {
        return -1;
    }

    ImGuiIO& io = ImGui::GetIO();
    LoadCustomIconFonts(io);

    Touch_Init(displayInfo.width, displayInfo.height, displayInfo.orientation, true);

    std::thread(VolumeKeyListenerThread).detach();
    InitTouchAimbot();

    while (main_thread_flag) {
        static auto lastProcessCheck = std::chrono::steady_clock::now();
        auto currentCheckTime = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentCheckTime - lastProcessCheck).count() >= 1000) {
            lastProcessCheck = currentCheckTime;

            if (game_pid <= 0) {
                game_pid = getProcessID(packageName);
                if (game_pid > 0) {
                    Memory::g_pid = (pid_t)game_pid;
                    driver->initialize(game_pid);
                    lib_base = 0;
                    lib_base2 = 0;
                }
            } else {
                if (!isProcessAlive(game_pid)) {
                    game_pid = 0;
                    lib_base = 0;
                    lib_base2 = 0;
                }
            }

            if (game_pid > 0 && lib_base == 0) {
                lib_base = get_module_base(targetaLibName, PERM_EXEC);
                if (lib_base > 0) {
                    lib_base2 = get_module_base(targetaLibName2, PERM_EXEC);
                }
            }
        }

        drawBegin();
        {
            if (showMenu) {
                RenderMenu();
            }

            if (game_pid > 0 && lib_base > 0) {
                DrawESP(abs_ScreenX, abs_ScreenY, lib_base);
                AimbotMenu(abs_ScreenX, abs_ScreenY, lib_base);
                TouchAimbotTick(showMenu);
            }
        }
        drawEnd();
        usleep(16000);
    }

    shutdown();
    Touch_Close();
    return 0;
}
