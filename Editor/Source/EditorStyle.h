#pragma once
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector2.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

namespace Fussion {
    class Texture2D;
}

constexpr auto AccentColor = 0x568af2FF;

struct ImFont;

enum class EditorFont {
    None = 0,
    RegularNormal,
    RegularBig,
    RegularSmall,
    RegularHuge,

    Bold,
    BoldSmall,
};

struct CommonStyle {
    Vector2 Padding{ 2, 2 };
    f32 Rounding{ 1.0f };
    bool Border{ false };
};

struct InteractiveStyle {
    Color NormalColor;
    Color HoverColor;
    Color PressedColor;

    constexpr InteractiveStyle()
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
    ButtonStyleImageButton,
    ButtonStyleViewportButton,

    ButtonStyleProjectCreator,
    ButtonStyleProjectCreatorSmall,

    ButtonStyleCount,
};

struct ButtonStyle : CommonStyle, InteractiveStyle {
    Color TextColor{};
    Color BorderColor{};
    Color BorderShadowColor{};
    EditorFont Font{};

    static constexpr ButtonStyle Default()
    {
        ButtonStyle style;
        style.Padding = Vector2(3, 3);
        style.NormalColor = Color::FromHex(AccentColor);
        style.TextColor = Color::White;
        style.BorderColor = style.NormalColor.Darken(0.1f);
        style.BorderShadowColor = Color::Transparent;
        style.Border = true;
        style.Rounding = 2.0f;
        return style;
    }

    constexpr void SetButtonColor(Color color)
    {
        NormalColor = color;
        HoverColor = NormalColor.Lighten(0.1f);
        PressedColor = NormalColor.Darken(0.1f);
        BorderColor = PressedColor;
        BorderShadowColor = HoverColor;
    }
};

enum WindowStyles {
    WindowStyleGeneric,
    WindowStyleCreator,
    WindowStyleAssetPreview,

    WindowStyleCount,
};

struct WindowStyle : CommonStyle {
    constexpr WindowStyle(): CommonStyle()
    {
        Border = true;
        Rounding = 3.0f;
    }
};


enum class EditorIcon {
    // ContentBrowser
    Folder = 0,
    FolderBack,
    GenericAsset,
    Scene,
    Script,
    PbrMaterial,

    // Other
    Dots,
    Search,

    Play,
    Stop,
    Pause,
    StepFrame,

    Error,
    Warning,
    Info,
    CogWheel,
};

struct EditorStyle {
    std::unordered_map<EditorFont, ImFont*> Fonts;

    std::array<ButtonStyle, ButtonStyleCount> ButtonStyles;
    std::array<WindowStyle, WindowStyleCount> WindowStyles;

    std::unordered_map<EditorIcon, Ref<Fussion::Texture2D>> EditorIcons;

    void Initialize();

    static EditorStyle& GetStyle();
};
