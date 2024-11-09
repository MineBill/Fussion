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
    editor_icons[Folder] = TextureLoader::load_texture_from_file("Assets/Icons/Folder.png").unwrap();
    editor_icons[FolderBack] = TextureLoader::load_texture_from_file("Assets/Icons/FolderBack.png").unwrap();
    editor_icons[GenericAsset] = TextureLoader::load_texture_from_file("Assets/Icons/GenericAsset.png").unwrap();
    editor_icons[Scene] = TextureLoader::load_texture_from_file("Assets/Icons/Scene.png").unwrap();
    editor_icons[Script] = TextureLoader::load_texture_from_file("Assets/Icons/Script.png").unwrap();
    editor_icons[PbrMaterial] = TextureLoader::load_texture_from_file("Assets/Icons/PbrMaterial.png").unwrap();
    editor_icons[Dots] = TextureLoader::load_texture_from_file("Assets/Icons/ThreeDots.png").unwrap();
    editor_icons[Search] = TextureLoader::load_texture_from_file("Assets/Icons/Search.png").unwrap();

    editor_icons[Error] = TextureLoader::load_texture_from_file("Assets/Icons/ErrorIcon.png").unwrap();
    editor_icons[Warning] = TextureLoader::load_texture_from_file("Assets/Icons/WarningIcon.png").unwrap();
    editor_icons[Info] = TextureLoader::load_texture_from_file("Assets/Icons/InfoIcon.png").unwrap();
    editor_icons[CogWheel] = TextureLoader::load_texture_from_file("Assets/Icons/CogWheel.png").unwrap();
    editor_icons[Entity] = TextureLoader::load_texture_from_file("Assets/Icons/Entity.png").unwrap();

    editor_icons[Play] = TextureLoader::load_texture_from_file("Assets/Icons/PlayButton.png").unwrap();
    editor_icons[Stop] = TextureLoader::load_texture_from_file("Assets/Icons/StopButton.png").unwrap();
    editor_icons[Pause] = TextureLoader::load_texture_from_file("Assets/Icons/PauseButton.png").unwrap();
    editor_icons[StepFrame] = TextureLoader::load_texture_from_file("Assets/Icons/StepFrame.png").unwrap();
}

EditorStyle& EditorStyle::style()
{
    return g_Style;
}
