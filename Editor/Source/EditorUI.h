#pragma once
#include "Layers/Editor.h"

namespace EUI {
void Property(auto&& func) {}

void Button(const char* label, auto&& func, Vector2 size = Vector2(), f32 alignment = 0)
{
    auto style = Editor::Get().GetStyle().ButtonStyles[ButtonStyleGeneric];

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1 : 0);

    ImGui::PushStyleColor(ImGuiCol_Button, style.BackgroundColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

    ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
    ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

    auto s = ImGui::CalcTextSize(label).x + style.Padding.X * 2.0f;
    auto avail = ImGui::GetContentRegionAvail().x;
    auto off = (avail - s) * alignment;

    if (off > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    }

    bool opened = ImGui::Button(label);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);

    if (opened) {
        func();
    }
}

void Window(const char* title, auto&& func, bool* opened = nullptr, ImGuiWindowFlags flags = 0)
{
    auto style = Editor::Get().GetStyle().WindowStyles[WindowStyleGeneric];

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.Border ? 1 : 0);

    bool o = ImGui::Begin(title, opened, flags);
    ImGui::PopStyleVar(3);

    if (o) {
        func();
    }

    ImGui::End();
}
}
