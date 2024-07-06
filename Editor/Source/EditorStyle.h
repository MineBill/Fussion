#pragma once
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector2.h"

#include <imgui.h>

constexpr auto AccentColor = 0x3C3798FF;

struct ImFont;


struct CommonStyle {
    Vector2 Padding{ 2, 2 };
    f32 Rounding{ 1.0f };
    bool Border{ false };
};

struct InteractiveStyle {
    Color NormalColor;
    Color HoverColor;
    Color PressedColor;

    InteractiveStyle()
    {
        auto accent = Color::FromHex(AccentColor);
        NormalColor = accent;
        HoverColor = accent.Lighten(0.1f);
        PressedColor = accent.Darken(0.1f);
    }
};

enum ButtonStyles {
    ButtonStyleGeneric,
    ButtonStyleDisabled,

    ButtonStyleCount,
};

struct ButtonStyle : CommonStyle, InteractiveStyle {
    Color TextColor{};
    Color BorderColor{};
    Color BorderShadowColor{};
    Color BackgroundColor{};

    static ButtonStyle Default()
    {
        ButtonStyle style;
        auto& imgui_style = ImGui::GetStyle();

        style.Padding = Vector2(3, 3);
        style.BackgroundColor = Color::FromHex(AccentColor);
        style.TextColor = CAST(Vector4, imgui_style.Colors[ImGuiCol_Text]);
        style.BorderColor = Color::White;
        style.BorderShadowColor = Color::Black;
        style.Rounding = 2.0f;
        return style;
    }
};

enum WindowStyles {
    WindowStyleGeneric,

    WindowStyleCount,
};

struct WindowStyle : CommonStyle {
    WindowStyle(): CommonStyle()
    {
        Border = true;
        Rounding = 3.0f;
    }
};

struct EditorFonts {
    ImFont* RegularNormal{ nullptr };
    ImFont* RegularSmall{ nullptr };
    ImFont* RegularHuge{ nullptr };

    ImFont* Bold{ nullptr };
    ImFont* BoldSmall{ nullptr };
};

struct EditorStyle {
    static EditorStyle Default();

    EditorFonts Fonts;

    std::array<ButtonStyle, ButtonStyleCount> ButtonStyles{};
    std::array<ButtonStyle, WindowStyleCount> WindowStyles{};
};
