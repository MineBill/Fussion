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
    Vector2 padding{ 2, 2 };
    f32 rounding{ 1.0f };
    bool border{ false };
};

struct InteractiveStyle {
    Color normal_color;
    Color hover_color;
    Color pressed_color;

    constexpr InteractiveStyle()
    {
        auto accent = Color::from_hex(AccentColor);
        normal_color = accent;
        hover_color = accent.lighten(0.1f);
        pressed_color = accent.darken(0.1f);
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
    Color text_color{};
    Color border_color{};
    Color border_shadow_color{};
    EditorFont font{};

    static constexpr ButtonStyle make_default()
    {
        ButtonStyle style;
        style.padding = Vector2(3, 3);
        style.normal_color = Color::from_hex(AccentColor);
        style.text_color = Color::White;
        style.border_color = style.normal_color.darken(0.1f);
        style.border_shadow_color = Color::Transparent;
        style.border = true;
        style.rounding = 2.0f;
        return style;
    }

    constexpr void set_button_color(Color color)
    {
        normal_color = color;
        hover_color = normal_color.lighten(0.1f);
        pressed_color = normal_color.darken(0.1f);
        border_color = pressed_color;
        border_shadow_color = hover_color;
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
        border = true;
        rounding = 3.0f;
        padding = { 4, 4 };
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
    Entity,
};

struct EditorStyle {
    std::unordered_map<EditorFont, ImFont*> fonts;

    std::array<ButtonStyle, ButtonStyleCount> button_styles;
    std::array<WindowStyle, WindowStyleCount> window_styles;

    std::unordered_map<EditorIcon, Ref<Fussion::Texture2D>> editor_icons;

    void initialize();

    static EditorStyle& get_style();
};
