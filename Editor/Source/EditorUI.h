#pragma once
#include "Layers/ImGuiLayer.h"
#include "EditorStyle.h"
#include "Layers/Editor.h"
#include "Project/Project.h"

#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Maybe.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/AssetRef.h"

#include <misc/cpp/imgui_stdlib.h>
#include <string>

namespace EUI {
    namespace Detail {
        ButtonStyle& GetButtonStyle(ButtonStyles style);
        WindowStyle& GetWindowStyle(WindowStyles style);

        template<class T, template<class...> class U>
        inline constexpr bool IsInstanceOf = std::false_type{};

        template<template<class...> class U, class... Vs>
        inline constexpr bool IsInstanceOf<U<Vs...>, U> = std::true_type{};
    }

    struct PropTypeGeneric {};

    struct PropTypeRange {
        f32 Min{}, Max{};
    };

    struct ImageButtonParams {
        ButtonStyles StyleType = ButtonStyleImageButton;
        Maybe<f32> Alignment;
        Maybe<Vector2> Size;
        bool Disabled = false;
    };

    /// Draws a button with a texture.
    /// @param image The texture to use.
    /// @param func The callback to call when the button is pressed.
    void ImageButton(Ref<Fussion::RHI::Image> const& image, auto&& func, ImageButtonParams params = {})
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

        auto size = params.Size.ValueOr(Vector2(0, 0));
        auto s = size.X + style.Padding.X * 2.0f;
        auto avail = ImGui::GetContentRegionAvail().x;
        auto off = (avail - s) * params.Alignment.ValueOr(0.0f);

        if (off > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }

        if (params.Disabled)
            ImGui::BeginDisabled();

        bool pressed = ImGui::ImageButton(IMGUI_IMAGE(image), size);
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(6);

        if (pressed) {
            func();
        }

        if (params.Disabled)
            ImGui::EndDisabled();
    }

    void ImageButton(Ref<Fussion::Texture2D> const& texture, auto&& func, ImageButtonParams params = {})
    {
        ImageButton(texture->GetImage(), func, params);
    }

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
        } else if constexpr (Detail::IsInstanceOf<T, Fussion::AssetRef>) {
            auto class_type = meta_hpp::resolve_type<T>();
            auto m_Handle = class_type.get_member("m_Handle");

            ImGui::TextUnformatted("Asset Reference:");
            ImGui::SetNextItemAllowOverlap();
            Vector2 pos = ImGui::GetCursorPos();

            auto handle = m_Handle.get(data).template as<Fussion::AssetHandle>();
            auto asset_metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(handle);
            ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::BoldSmall]);
            ImGui::Button(std::format("{}", asset_metadata.Name.data()).data(), Vector2(64, 64));
            ImGui::PopFont();

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Clear")) {
                    m_Handle.set(data, Fussion::AssetHandle(0));
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropTarget()) {
                auto* payload = ImGui::GetDragDropPayload();
                if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                    auto incoming_handle = CAST(Fussion::AssetHandle*, payload->Data);
                    auto incoming_metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(*incoming_handle);

                    if (incoming_metadata.Type == asset_metadata.Type && ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        m_Handle.set(data, *incoming_handle);
                    }
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::SetCursorPos(pos + Vector2(2, 2));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(0, 0));
            EUI::ImageButton(EditorStyle::GetStyle().EditorIcons[EditorIcon::Search], [&] {
                auto asset_type = class_type.get_method("GetType").invoke(data).template as<Fussion::AssetType>();
                Editor::GenericAssetPicker.Show(m_Handle, data, asset_type);
            }, { .Size = Vector2{ 16, 16 } });
            ImGui::PopStyleVar();
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

    struct ButtonParams {
        ButtonStyles Style{ ButtonStyleGeneric };
        f32 Alignment{ 0.0f };
        bool Disabled{ false };
        Maybe<Vector2> Size{};
        Maybe<ButtonStyle> Override{};
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

        auto const& style = params.Override.ValueOr(Detail::GetButtonStyle(params.Style));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.Rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.Border ? 1.f : 0.f);

        ImGui::PushStyleColor(ImGuiCol_Button, style.NormalColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.HoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.PressedColor);

        ImGui::PushStyleColor(ImGuiCol_Text, style.TextColor);
        ImGui::PushStyleColor(ImGuiCol_Border, style.BorderColor);
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, style.BorderShadowColor);

        ImGui::PushFont(EditorStyle::GetStyle().Fonts[style.Font]);

        auto s = !params.Size.HasValue() ? ImGui::CalcTextSize(label.data()).x : params.Size->X + style.Padding.X * 2.0f;
        // auto s = params.Size.ValueOr(Vector2(ImGui::CalcTextSize(label.data()).x, 0)).X + style.Padding.X * 2.0f;
        auto avail = ImGui::GetContentRegionAvail().x;
        auto off = (avail - s) * params.Alignment;
        if (off > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }

        if (params.Disabled)
            ImGui::BeginDisabled();

        bool opened = ImGui::Button(label.data(), params.Size.ValueOr(Vector2::Zero));

        if (params.Disabled)
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
                return std::optional<ResultType>{ func() };
            }
            return std::optional<ResultType>{};
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
        ImGuiWindowFlags Flags = 0;
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
        WindowStyles Style = WindowStyleGeneric;
        bool* Opened{ nullptr };
        ImGuiWindowFlags Flags = 0;
        Vector2 Size{ 400, 400 };
        bool Dirty = false;
    };

    void Window(std::string_view title, auto&& func, WindowParams params = {})
    {
        auto style = Detail::GetWindowStyle(params.Style);

        if (params.Dirty)
            params.Flags |= ImGuiWindowFlags_UnsavedDocument;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.Padding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.Rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.Border ? 1.f : 0.f);

        if (!params.Size.IsZero())
            ImGui::SetNextWindowSize(params.Size, ImGuiCond_Appearing);
        bool o = ImGui::Begin(title.data(), params.Opened, params.Flags);
        ImGui::PopStyleVar(3);

        if (o) {
            func();
        }

        ImGui::End();
    }
}
