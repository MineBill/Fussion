﻿#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGuiHelpers.h"

#include "Engin5/Core/Core.h"
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Layers/ImGuiLayer.h"

static ImVector<ImRect> s_GroupPanelLabelStack;

void ImGuiHelpers::BeginProperty(const char* label)
{
    const auto table_flags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_SizingStretchSame;
    ImGui::BeginTable(label, 2, table_flags);
}

void ImGuiHelpers::EndProperty()
{
    ImGui::EndTable();
}

void ImGuiHelpers::BeginGroupPanel(const char *name, const ImVec2 &size, ImFont* font)
{
    ImGui::PushFont(font);
    defer (ImGui::PopFont());

    ImGui::BeginGroup();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(2.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vector2(2.0f, 2.0f));

    auto frameHeight = ImGui::GetFrameHeight();
    ImGui::BeginGroup();

    ImVec2 effectiveSize = size;
    if (size.x < 0.0f)
        effectiveSize.x = ImGui::GetContentRegionAvail().x;
    else
        effectiveSize.x = size.x;
    ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextUnformatted(name);
    auto labelMin = ImGui::GetItemRectMin();
    auto labelMax = ImGui::GetItemRectMax();
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
    ImGui::BeginGroup();

    // ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x -= frameHeight;

    auto itemWidth = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

    s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
}

void ImGuiHelpers::EndGroupPanel()
{
    ImGui::PopItemWidth();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));

    auto frameHeight = ImGui::GetFrameHeight();

    ImGui::EndGroup();

    // ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255,
    // 0, 64), 4.0f);

    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

    ImGui::EndGroup();

    auto itemMin = ImGui::GetItemRectMin();
    auto itemMax = ImGui::GetItemRectMax();
    // ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

    auto labelRect = s_GroupPanelLabelStack.back();
    s_GroupPanelLabelStack.pop_back();

    ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
    ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
    labelRect.Min.x -= itemSpacing.x;
    labelRect.Max.x += itemSpacing.x;
    for (int i = 0; i < 4; ++i) {
        switch (i) {
        // left half-plane
        case 0:
            ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true);
            break;
        // right half-plane
        case 1:
            ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true);
            break;
        // top
        case 2:
            ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true);
            break;
        // bottom
        case 3:
            ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true);
            break;
        }

        ImGui::GetWindowDrawList()->AddRect(frameRect.Min, frameRect.Max,
                                            ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)), halfFrame.x);

        ImGui::PopClipRect();
    }

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x += frameHeight;

    ImGui::Dummy(ImVec2(0.0f, 0.0f));

    ImGui::EndGroup();
}

void ImGuiHelpers::DragVec3(const char *id, glm::vec3 *value, f32 speed, f32 min, f32 max, const char *format, ImFont* font, ImFont* font2)
{
    ImGui::PushID(id);
    constexpr auto X_COLOR = ImVec4(0.92f, 0.24f, 0.27f, 1.0);
    constexpr auto X_COLOR_HOVER = ImVec4(0.76f, 0.20f, 0.22f, 1.0);
    constexpr auto Y_COLOR = ImVec4(0.20f, 0.67f, 0.32f, 1.0);
    constexpr auto Y_COLOR_HOVER = ImVec4(0.15f, 0.52f, 0.25f, 1.0);
    constexpr auto Z_COLOR = ImVec4(0.18f, 0.49f, 0.74f, 1.0);
    constexpr auto Z_COLOR_HOVER = ImVec4(0.14f, 0.39f, 0.60f, 1.0);
    constexpr s32 SPACE_COUNT = 5;
    constexpr f32 ITEM_COUNT = 3;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
    auto spacing = ImGui::GetStyle().ItemSpacing.x;
    auto width =
        (ImGui::GetContentRegionAvail().x - spacing * SPACE_COUNT - ImGui::CalcTextSize("X").x * 3) / ITEM_COUNT;

    ImGui::AlignTextToFramePadding();

    auto DrawLabel = [font](const char* label, ImVec4 color) {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::PushFont(font);
        ImGui::Text(label);
        ImGui::PopFont();
        ImGui::PopStyleColor();
    };

    DrawLabel("X", X_COLOR);
    ImGui::SameLine();
    ImGui::PushItemWidth(width);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, X_COLOR);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, X_COLOR_HOVER);
    ImGui::PushFont(font2);
    ImGui::DragFloat("##x", &value->x, speed, min, max, format);
    ImGui::PopFont();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    DrawLabel("Y", Y_COLOR);

    ImGui::SameLine();
    ImGui::PushItemWidth(width);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Y_COLOR);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Y_COLOR_HOVER);
    ImGui::PushFont(font2);
    ImGui::DragFloat("##y", &value->y, speed, min, max, format);
    ImGui::PopFont();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    DrawLabel("Z", Z_COLOR);

    ImGui::SameLine();
    ImGui::PushItemWidth(width);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Z_COLOR);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Z_COLOR_HOVER);
    ImGui::PushFont(font2);
    ImGui::DragFloat("##z", &value->z, speed, min, max, format);
    ImGui::PopFont();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::PopStyleVar();
    ImGui::PopID();
}

bool ImGuiHelpers::ButtonCenteredOnLine(const char *label, float alignment)
{
    ImGuiStyle &style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::Button(label);
}

void ImGuiHelpers::RenderSimpleRect(ImDrawList *draw_list, glm::vec2 const &position, glm::vec2 const &size, u32 color,
                                    f32 width)
{
    ImGui::RenderRectFilledWithHole(
        draw_list, ImRect(position.x, position.y, position.x + size.x, position.y + size.y),
        ImRect(position.x + width, position.y + width, position.x + size.x - width, position.y + size.y - width), color,
        0.0f);
}

void ImGuiHelpers::InputText(const char* label, std::string& value, ImGuiInputTextFlags flags)
{
    BeginProperty(label);
    defer (EndProperty());

    if (flags & ImGuiInputTextFlags_ReadOnly)
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushID(label);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##input", &value, flags);
    ImGui::PopID();
}

bool ImGuiHelpers::ImageToggleButton(const char* id, Ref<Engin5::Image> const& image, bool& toggled, Vector2 size)
{
    auto button = cast(Vector4, ImGui::GetStyleColorVec4(ImGuiCol_Button)) * 0.8f;
    // auto button = Vector4(0, 1, 0, 1);

    if (toggled) {
        ImGui::PushStyleColor(ImGuiCol_Button, button);
    }
    const auto pressed = ImGui::ImageButton(id, IMGUI_IMAGE(image), size);
    if (toggled) {
        ImGui::PopStyleColor();
    }

    if (pressed) {
        toggled = !toggled;
    }
    return pressed;
}