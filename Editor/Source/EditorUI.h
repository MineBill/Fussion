﻿#pragma once
#include "EditorStyle.h"
#include "Layers/Editor.h"
#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"

#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/Texture2D.h>
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Maybe.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

namespace EUI {
    namespace Detail {
        ButtonStyle& GetButtonStyle(ButtonStyles style);
        WindowStyle& GetWindowStyle(WindowStyles style);
    }

    struct PropTypeGeneric { };

    struct PropTypeRange {
        f32 min {}, max {};
    };

    struct ImageButtonParams {
        ButtonStyles style_type = ButtonStyleImageButton;
        Maybe<f32> alignment {};
        Maybe<Vector2> size {};
        bool disabled = false;
    };

    /// Draws a button with a texture.
    /// @param texture The texture to use.
    /// @param func The callback to call when the button is pressed.
    void ImageButton(Fussion::GPU::TextureView const& texture, auto&& func, ImageButtonParams params = {})
    {
        auto style = Detail::GetButtonStyle(params.style_type);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1.f : 0.f);

        ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

        ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
        ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

        auto size = params.size.ValueOr(Vector2(0, 0));
        auto s = size.x + style.Padding.x * 2.0f;
        auto avail = ImGui::GetContentRegionAvail().x;
        auto off = (avail - s) * params.alignment.ValueOr(0.0f);

        if (off > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }

        if (params.disabled)
            ImGui::BeginDisabled();

        bool pressed = ImGui::ImageButton(texture, size);
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(6);

        if (pressed) {
            func();
        }

        if (params.disabled)
            ImGui::EndDisabled();
    }

    void ImageButton(Ref<Fussion::Texture2D> const& texture, auto&& func, ImageButtonParams params = {})
    {
        ImageButton(texture->GetTexture().View, func, params);
    }

    bool AssetProperty(meta_hpp::class_type class_type, meta_hpp::uvalue data);

    template<typename T, typename TypeKind = PropTypeGeneric>
    bool Property(std::string_view name, T* data, TypeKind kind = {})
    {
        bool modified { false };
        constexpr auto table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchSame;
        auto table_opened = ImGui::BeginTable(name.data(), 2, table_flags);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(name.data());

        ImGui::TableNextColumn();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        defer(ImGui::PopStyleVar());

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>) {
            if constexpr (std::is_same_v<TypeKind, PropTypeRange>) {
                modified |= ImGui::SliderFloat("", data, kind.min, kind.max);
            } else {
                modified |= ImGui::InputFloat("", data);
            }
        } else if constexpr (std::is_same_v<T, s32> || std::is_same_v<T, s64>) {
            if constexpr (std::is_same_v<TypeKind, PropTypeRange>) {
                modified |= ImGui::SliderInt("", data, CAST(T, kind.min), CAST(T, kind.max));
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
            modified |= ImGui::ColorEdit4("", data->raw);
        } else if constexpr (std::is_enum_v<T>) {
            auto opened = ImGui::BeginCombo("", magic_enum::enum_name(*data).data());
            if (opened) {
                for (auto const& [evalue, ename] : magic_enum::enum_entries<T>()) {
                    if (ImGui::Selectable(ename.data())) {
                        *data = evalue;
                        modified |= true;
                    }
                }
                ImGui::EndCombo();
            }
        } else if constexpr (Fussion::IsInstanceOf<T, Fussion::AssetRef>) {
            auto class_type = meta_hpp::resolve_type<T>();
            AssetProperty(class_type, data);
        } else if constexpr (std::is_same_v<T, Vector2>) {
            modified |= ImGui::InputFloat2("", data->raw);
        } else if constexpr (std::is_same_v<T, Vector3>) {
            modified |= ImGui::InputFloat3("", data->raw);
        } else if constexpr (std::is_same_v<T, Vector4>) {
            modified |= ImGui::InputFloat4("", data->raw);
        } else {
            static_assert(false, "Not implemented!");
        }

        if (table_opened) {
            ImGui::EndTable();
        }

        return modified;
    }

    void Property(std::string_view name, auto&& data)
    {
        constexpr auto table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchSame;
        auto table_opened = ImGui::BeginTable(name.data(), 2, table_flags);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(name.data());

        ImGui::TableNextColumn();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        data();

        if (table_opened) {
            ImGui::EndTable();
        }
    }

    struct ButtonParams {
        ButtonStyles style { ButtonStyleGeneric };
        f32 alignment { 0.0f };
        bool disabled { false };
        Maybe<Vector2> size {};
        Maybe<ButtonStyle> override {};
    };

    /// Draws a button.
    /// @param label The button name.
    /// @param func A callback that will be called if the button was pressed.
    /// @param params Parameters.
    /// @return If the @p func return type is non-void, then this function will return whatever @p func returns.
    template<typename Func>
    auto Button(std::string_view label, Func&& func, ButtonParams params = {})
    {
        using ResultType = std::invoke_result_t<Func>;

        auto const& style = params.override.ValueOr(Detail::GetButtonStyle(params.style));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1.f : 0.f);

        ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

        ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
        ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

        ImGui::PushFont(EditorStyle::Style().Fonts[style.Font]);

        auto s = !params.size.HasValue() ? ImGui::CalcTextSize(label.data()).x : params.size->x + style.Padding.x * 2.0f;
        // auto s = params.Size.ValueOr(Vector2(ImGui::CalcTextSize(label.data()).x, 0)).X + style.Padding.X * 2.0f;
        auto avail = ImGui::GetContentRegionAvail().x;
        auto off = (avail - s) * params.alignment;
        if (off > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }

        if (params.disabled)
            ImGui::BeginDisabled();

        bool opened = ImGui::Button(label.data(), params.size.ValueOr(Vector2::Zero));

        if (params.disabled)
            ImGui::EndDisabled();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(6);

        ImGui::PopFont();

        if constexpr (std::is_void_v<ResultType>) {
            if (opened) {
                func();
            }
        } else {
            if (opened) {
                return std::optional<ResultType> { func() };
            }
            return std::optional<ResultType> {};
        }
    }

    void Popup(std::string_view title, auto&& func)
    {
        bool opened = ImGui::BeginPopup(title.data());
        if (opened) {
            func();
            ImGui::EndPopup();
        }
    }

    struct ModalWindowParams {
        ImGuiWindowFlags flags = 0;
        bool* opened { nullptr };
    };

    template<typename Func>
    auto ModalWindow(std::string_view title, Func&& func, ModalWindowParams params = {})
    {
        using ResultType = std::invoke_result_t<Func>;
        bool opened = ImGui::BeginPopupModal(title.data(), params.opened, params.flags);

        if constexpr (std::is_void_v<ResultType>) {
            if (opened) {
                func();
                ImGui::EndPopup();
            }
        } else {
            if (opened) {
                auto&& ret = func();
                ImGui::EndPopup();
                return std::optional<ResultType> { ret };
            }
            return std::optional<ResultType> {};
        }
    }

    struct WindowParams {
        WindowStyles style { WindowStyleGeneric };
        bool* opened { nullptr };
        ImGuiWindowFlags flags { ImGuiWindowFlags_None };
        Vector2 size { 400, 400 };
        bool dirty { false };
        bool centered { false };
        bool use_child { true };
        Maybe<WindowStyle> override {};
    };

    void Window(std::string_view title, auto&& func, WindowParams params = {})
    {
        auto style = params.override.ValueOr(Detail::GetWindowStyle(params.style));

        if (params.dirty)
            params.flags |= ImGuiWindowFlags_UnsavedDocument;
        if (params.centered) {
            auto& io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.Padding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.Rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.Border ? 1.f : 0.f);

        if (!params.size.IsZero())
            ImGui::SetNextWindowSize(params.size, ImGuiCond_Appearing);
        bool o = ImGui::Begin(title.data(), params.opened, params.flags);
        ImGui::PopStyleVar(3);

        if (params.use_child) {
            ImGui::BeginChild("##inner_child", {}, ImGuiChildFlags_Border);
        }
        if (o) {
            func();
        }
        if (params.use_child) {
            ImGui::EndChild();
        }

        ImGui::End();
    }

    void WithFont(ImFont* font, auto&& callback)
    {
        ImGui::PushFont(font);
        callback();
        ImGui::PopFont();
    }

    void WithEditorFont(EditorFont font, auto&& callback)
    {
        auto* f = EditorStyle::Style().Fonts[font];
        ImGui::PushFont(f);
        callback();
        ImGui::PopFont();
    }
}
