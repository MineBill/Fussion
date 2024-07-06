#pragma once
#include "Layers/Editor.h"
#include "Layers/ImGuiLayer.h"
#include <string>

namespace EUI {
struct PropTypeGeneric {};

struct PropTypeRange {
    f32 Min{}, Max{};
};

template<typename T, typename TypeKind = PropTypeGeneric>
void Property(std::string const& name, T* data, TypeKind kind = {})
{
    constexpr auto table_flags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_SizingStretchSame;
    ImGui::BeginTable(name.c_str(), 2, table_flags);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(name.c_str());

    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>) {
        if constexpr (std::is_same_v<TypeKind, PropTypeRange>) {
            ImGui::SliderFloat("", data, kind.Min, kind.Max);
        } else {
            ImGui::InputFloat("", data);
        }
    } else if constexpr (std::is_same_v<T, s32> || std::is_same_v<T, s64>) {
        if constexpr (std::is_same_v<TypeKind, PropTypeRange>) {
            ImGui::SliderInt("", data, CAST(T, kind.Min), CAST(T, kind.Max));
        } else {
            ImGui::InputInt("", data);
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        ImGui::Checkbox("", data);
    } else {
        static_assert(false, "Not implemented!");
    }

    ImGui::EndTable();
}

void Property(std::string const& name, auto&& data)
{
    using Type = std::remove_reference_t<decltype(data)>;

    constexpr auto table_flags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_SizingStretchSame;
    ImGui::BeginTable(name.c_str(), 2, table_flags);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(name.c_str());

    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    data();

    ImGui::EndTable();
}

void Button(std::string const& label, auto&& func, ButtonStyles style_type = ButtonStyleGeneric, Vector2 size = Vector2(), f32 alignment = 0)
{
    auto style = Editor::Get().GetStyle().ButtonStyles[style_type];

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1 : 0);

    ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

    ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
    ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

    auto s = ImGui::CalcTextSize(label.c_str()).x + style.Padding.X * 2.0f;
    auto avail = ImGui::GetContentRegionAvail().x;
    auto off = (avail - s) * alignment;
    if (off > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    }

    bool opened = ImGui::Button(label.c_str());
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);

    if (opened) {
        func();
    }
}

void ImageButton(Ref<Fussion::Texture2D> const& texture, Vector2 size, auto&& func, ButtonStyles style_type = ButtonStyleImageButton, f32 alignment = 0.0f)
{
    auto style = Editor::Get().GetStyle().ButtonStyles[style_type];

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1 : 0);

    ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

    ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
    ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

    auto s = size.X + style.Padding.X * 2.0f;
    auto avail = ImGui::GetContentRegionAvail().x;
    auto off = (avail - s) * alignment;

    if (off > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    }

    bool pressed = ImGui::ImageButton(IMGUI_IMAGE(texture->GetImage()), size);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);

    if (pressed) {
        func();
    }
}

void Popup(const char* title, auto&& func)
{
    bool opened = ImGui::BeginPopup(title);
    if (opened) {
        func();
        ImGui::EndPopup();
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
