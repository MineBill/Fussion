#pragma once
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

    struct ImGuiStyleBuilder {
        ~ImGuiStyleBuilder()
        {
            ImGui::PopStyleVar(m_StyleCount);
            ImGui::PopStyleColor(m_ColorCount);
        }

        ImGuiStyleBuilder& With(ImGuiStyleVar style, f32 val)
        {
            ImGui::PushStyleVar(style, val);
            m_StyleCount++;
            return *this;
        }

        ImGuiStyleBuilder& With(ImGuiStyleVar style, Vector2 const& val)
        {
            ImGui::PushStyleVar(style, val);
            m_StyleCount++;
            return *this;
        }

        ImGuiStyleBuilder& With(ImGuiCol color, Color const& val, bool cond = true)
        {
            if (cond) {
                ImGui::PushStyleColor(color, val);
                m_ColorCount++;
            }
            return *this;
        }

        template<typename F>
        auto Do(F callback)
        {
            return callback();
        }

    private:
        u32 m_StyleCount { 0 };
        u32 m_ColorCount { 0 };
    };

    struct PropTypeGeneric { };

    struct PropTypeRange {
        f32 Min {}, Max {};
    };

    struct ImageButtonParams {
        ButtonStyles Style = ButtonStyleImageButton;
        Maybe<f32> Alignment {};
        Maybe<Vector2> Size {};
        bool Disabled = false;
    };

    /// Draws a button with a texture.
    /// @param texture The texture to use.
    /// @param func The callback to call when the button is pressed.
    void ImageButton(Fussion::GPU::TextureView const& texture, auto&& func, ImageButtonParams params = {})
    {
        auto style = Detail::GetButtonStyle(params.Style);

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
        auto s = size.x + style.Padding.x * 2.0f;
        auto avail = ImGui::GetContentRegionAvail().x;
        auto off = (avail - s) * params.Alignment.ValueOr(0.0f);

        if (off > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }

        if (params.Disabled)
            ImGui::BeginDisabled();

        bool pressed = ImGui::ImageButton(texture, size);
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
        ButtonStyles Style { ButtonStyleGeneric };
        f32 Alignment { 0.0f };
        bool Disabled { false };
        Maybe<Vector2> Size {};
        Maybe<ButtonStyle> StyleOverride {};
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

        auto const& style = params.StyleOverride.ValueOr(Detail::GetButtonStyle(params.Style));
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

        auto s = !params.Size.HasValue() ? ImGui::CalcTextSize(label.data()).x : params.Size->x + style.Padding.x * 2.0f;
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
        ImGuiWindowFlags Flags = 0;
        bool* Opened { nullptr };
    };

    template<typename Func>
    auto ModalWindow(std::string_view title, Func func, ModalWindowParams params = {})
    {
        using ResultType = std::invoke_result_t<Func>;

        bool opened = ImGuiStyleBuilder()
                          .With(ImGuiCol_PopupBg, Color::Orange)
                          .Do([&] {
                              return ImGui::BeginPopupModal(title.data(), params.Opened, params.Flags);
                          });

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
        WindowStyles Style { WindowStyleGeneric };
        bool* Opened { nullptr };
        ImGuiWindowFlags Flags { ImGuiWindowFlags_None };
        Vector2 Size { 400, 400 };
        bool Dirty { false };
        bool Centered { false };
        bool UseChild { false };
        Maybe<WindowStyle> StyleOverride {};
    };

    void Window(std::string_view title, auto&& func, WindowParams params = {})
    {
        auto style = params.StyleOverride.ValueOr(Detail::GetWindowStyle(params.Style));

        if (params.Dirty)
            params.Flags |= ImGuiWindowFlags_UnsavedDocument;
        if (params.Centered) {
            auto& io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.Padding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.Rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.Border ? 1.f : 0.f);

        if (!params.Size.IsZero())
            ImGui::SetNextWindowSize(params.Size, ImGuiCond_Appearing);
        bool o = ImGui::Begin(title.data(), params.Opened, params.Flags);
        ImGui::PopStyleVar(3);

        if (params.UseChild) {
            ImGui::BeginChild("##inner_child", {}, ImGuiChildFlags_Border);
        }
        if (o) {
            func();
        }
        if (params.UseChild) {
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
