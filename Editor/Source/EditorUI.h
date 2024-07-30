#pragma once
#include "Layers/ImGuiLayer.h"
#include "EditorStyle.h"

#include "Fussion/Assets/Texture2D.h"
#include <misc/cpp/imgui_stdlib.h>
#include <string>

namespace EUI {
namespace Detail {
ButtonStyle& GetButtonStyle(ButtonStyles style);
WindowStyle& GetWindowStyle(WindowStyles style);
}

struct PropTypeGeneric {};

struct PropTypeRange {
    f32 Min{}, Max{};
};

template<typename T, typename TypeKind = PropTypeGeneric>
bool Property(std::string_view name, T* data, TypeKind kind = {})
{
    bool modified{ false };
    constexpr auto table_flags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_SizingStretchSame;
    ImGui::BeginTable(name.data(), 2, table_flags);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(name.data());

    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>) {
        if constexpr (std::is_same_v<TypeKind, PropTypeRange>) {
            modified |= ImGui::SliderFloat("", data, kind.Min, kind.Max);
        } else {
            modified |= ImGui::InputFloat("", data);
        }
    } else if constexpr (std::is_same_v<T, s32> || std::is_same_v<T, s64>) {
        if constexpr (std::is_same_v<TypeKind, PropTypeRange>) {
            modified |= ImGui::SliderInt("", data, CAST(T, kind.Min), CAST(T, kind.Max));
        } else {
            modified |= ImGui::InputInt("", data);
        }
    } else if constexpr (std::is_same_v<T, Fussion::Uuid>) {
        modified |= ImGui::InputScalar("", ImGuiDataType_U64, data);
    } else if constexpr (std::is_same_v<T, bool>) {
        modified |= ImGui::Checkbox("", data);
    } else if constexpr (std::is_same_v<T, std::string>) {
        modified |= ImGui::InputText("", data);
    } else if constexpr (std::is_same_v<T, Color>) {
        modified |= ImGui::ColorEdit4("", data->Raw);
    } else {
        static_assert(false, "Not implemented!");
    }

    ImGui::EndTable();

    return modified;
}

void Property(std::string_view name, auto&& data)
{
    constexpr auto table_flags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_SizingStretchSame;
    ImGui::BeginTable(name.data(), 2, table_flags);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(name.data());

    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    data();

    ImGui::EndTable();
}

template<typename Func>
auto Button(std::string_view label, Func&& func, ButtonStyles style_type = ButtonStyleGeneric, [[maybe_unused]] Vector2 size = Vector2(), f32 alignment = 0)
{
    using ResultType = std::invoke_result_t<Func>;
    auto style = Detail::GetButtonStyle(style_type);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1.f : 0.f);

    ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

    ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
    ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

    auto s = ImGui::CalcTextSize(label.data()).x + style.Padding.X * 2.0f;
    auto avail = ImGui::GetContentRegionAvail().x;
    auto off = (avail - s) * alignment;
    if (off > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    }

    bool opened = ImGui::Button(label.data());
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);

    if constexpr (std::is_void_v<ResultType>) {
        if (opened) {
            func();
        }
    } else {
        if (opened) {
            return std::optional<ResultType>{ func() };
        }
        return std::optional<ResultType>{};
    }
}

struct ImageButtonParams {
    ButtonStyles StyleType = ButtonStyleImageButton;
    f32 Alignment = 0.0f;
    bool Disabled = false;
};

void ImageButton(Ref<Fussion::Texture2D> const& texture, Vector2 size, auto&& func, ImageButtonParams params = {})
{
    auto style = Detail::GetButtonStyle(params.StyleType);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1.f : 0.f);

    ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

    ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
    ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

    auto s = size.X + style.Padding.X * 2.0f;
    auto avail = ImGui::GetContentRegionAvail().x;
    auto off = (avail - s) * params.Alignment;

    if (off > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    }

    if (params.Disabled)
        ImGui::BeginDisabled();

    bool pressed = ImGui::ImageButton(IMGUI_IMAGE(texture->GetImage()), size);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);

    if (pressed) {
        func();
    }

    if (params.Disabled)
        ImGui::EndDisabled();
}

void Popup(const char* title, auto&& func)
{
    bool opened = ImGui::BeginPopup(title);
    if (opened) {
        func();
        ImGui::EndPopup();
    }
}

struct ModalWindowParams {
    ImGuiPopupFlags Flags = 0;
    bool* Opened{ nullptr };
};

template<typename Func>
auto ModalWindow(std::string_view title, Func&& func, ModalWindowParams params = {})
{
    using ResultType = std::invoke_result_t<Func>;
    bool opened = ImGui::BeginPopupModal(title.data(), params.Opened, params.Flags);

    if constexpr (std::is_void_v<ResultType>) {
        if (opened) {
            func();
            ImGui::EndPopup();
        }
    } else {
        if (opened) {
            auto&& ret = func();
            ImGui::EndPopup();
            return std::optional<ResultType>{ ret };
        }
        return std::optional<ResultType>{};
    }
}

struct WindowParams {
    bool* Opened{ nullptr };
    ImGuiWindowFlags Flags = 0;
    Vector2 Size{ 400, 400 };
    bool Dirty = false;
};

void Window(const char* title, auto&& func, WindowParams params = {})
{
    auto style = Detail::GetWindowStyle(WindowStyleGeneric);

    if (params.Dirty)
        params.Flags |= ImGuiWindowFlags_UnsavedDocument;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.Border ? 1.f : 0.f);

    if (!params.Size.IsZero())
        ImGui::SetNextWindowSize(params.Size, ImGuiCond_Appearing);
    bool o = ImGui::Begin(title, params.Opened, params.Flags);
    ImGui::PopStyleVar(3);

    if (o) {
        func();
    }

    ImGui::End();
}
}
