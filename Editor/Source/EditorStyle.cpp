#include "EditorPCH.h"
#include "EditorStyle.h"

#include <Fussion/Util/TextureLoader.h>

#include <filesystem>

using namespace Fussion;

EditorStyle g_Style;

void EditorStyle::initialize()
{
    button_styles[ButtonStyleGeneric] = ButtonStyle::default_();
    button_styles[ButtonStyleDisabled] = ButtonStyle::default_();

    {
        auto style = ButtonStyle::default_();
        style.padding = Vector2::Zero;
        style.border = false;
        style.normal_color = Color::Transparent;
        style.hover_color = Color::Black;
        style.hover_color.a = 0.2f;

        style.pressed_color = Color::White;
        style.pressed_color.a = 0.2f;

        button_styles[ButtonStyleImageButton] = style;
    }

    {
        auto style = ButtonStyle::default_();
        style.border = false;
        style.rounding = 3.0f;

        style.SetButtonColor(Color::from_hex(ACCENT_COLOR));

        button_styles[ButtonStyleViewportButton] = style;
    }

    {
        auto style = ButtonStyle::default_();
        style.border = false;
        style.rounding = 3.0f;
        style.font = EditorFont::RegularBig;

        style.SetButtonColor(Color::from_hex(ACCENT_COLOR));

        button_styles[ButtonStyleProjectCreator] = style;
    }

    {
        auto style = ButtonStyle::default_();
        style.border = false;
        style.rounding = 3.0f;
        style.font = EditorFont::RegularNormal;

        style.SetButtonColor(Color::from_hex(ACCENT_COLOR));

        button_styles[ButtonStyleProjectCreatorSmall] = style;
    }

    {
        auto style = WindowStyle();
        style.padding = { 3, 3 };
        window_styles[WindowStyleGeneric] = style;
    }

    {
        auto style = WindowStyle();
        style.border = false;
        style.padding = { 10, 10 };
        style.rounding = 0.0;
        window_styles[WindowStyleCreator] = style;
    }

    {
        auto style = WindowStyle();
        style.border = true;
        style.padding = { 5, 5 };
        window_styles[WindowStyleAssetPreview] = style;
    }

    using enum EditorIcon;
    std::vector<std::pair<EditorIcon, std::string>> icon_paths = {
        { Folder, "Assets/Icons/Folder.png" },
        { FolderBack, "Assets/Icons/FolderBack.png" },
        { GenericAsset, "Assets/Icons/GenericAsset.png" },
        { Scene, "Assets/Icons/Scene.png" },
        { Script, "Assets/Icons/Script.png" },
        { PbrMaterial, "Assets/Icons/PbrMaterial.png" },
        { Dots, "Assets/Icons/ThreeDots.png" },
        { Search, "Assets/Icons/Search.png" },
        { Error, "Assets/Icons/ErrorIcon.png" },
        { Warning, "Assets/Icons/WarningIcon.png" },
        { Info, "Assets/Icons/InfoIcon.png" },
        { CogWheel, "Assets/Icons/CogWheel.png" },
        { Entity, "Assets/Icons/Entity.png" },
        { Play, "Assets/Icons/PlayButton.png" },
        { Stop, "Assets/Icons/StopButton.png" },
        { Pause, "Assets/Icons/PauseButton.png" },
        { StepFrame, "Assets/Icons/StepFrame.png" }
    };

    for (auto const& [icon_type, path] : icon_paths) {
        editor_icons[icon_type] = TextureLoader::load_texture_from_file(path, GPU::TextureFormat::RGBA8UnormSrgb).unwrap();
    }
}

EditorStyle& EditorStyle::style()
{
    return g_Style;
}
