#include "EditorPCH.h"
#include "EditorStyle.h"

#include <Fussion/Util/TextureImporter.h>

#include <filesystem>

using namespace Fussion;

EditorStyle g_Style;

void EditorStyle::initialize()
{
    button_styles[ButtonStyleGeneric] = ButtonStyle::make_default();
    button_styles[ButtonStyleDisabled] = ButtonStyle::make_default();

    {
        auto style = ButtonStyle::make_default();
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
        auto style = ButtonStyle::make_default();
        style.border = false;
        style.rounding = 3.0f;

        style.set_button_color(Color::from_hex(AccentColor));

        button_styles[ButtonStyleViewportButton] = style;
    }

    {
        auto style = ButtonStyle::make_default();
        style.border = false;
        style.rounding = 3.0f;
        style.font = EditorFont::RegularBig;

        style.set_button_color(Color::from_hex(AccentColor));

        button_styles[ButtonStyleProjectCreator] = style;
    }

    {
        auto style = ButtonStyle::make_default();
        style.border = false;
        style.rounding = 3.0f;
        style.font = EditorFont::RegularNormal;

        style.set_button_color(Color::from_hex(AccentColor));

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
    editor_icons[Folder] = TextureImporter::load_texture_from_file("Assets/Icons/Folder.png").value();
    editor_icons[FolderBack] = TextureImporter::load_texture_from_file("Assets/Icons/FolderBack.png").value();
    editor_icons[GenericAsset] = TextureImporter::load_texture_from_file("Assets/Icons/GenericAsset.png").value();
    editor_icons[Scene] = TextureImporter::load_texture_from_file("Assets/Icons/Scene.png").value();
    editor_icons[Script] = TextureImporter::load_texture_from_file("Assets/Icons/Script.png").value();
    editor_icons[PbrMaterial] = TextureImporter::load_texture_from_file("Assets/Icons/PbrMaterial.png").value();
    editor_icons[Dots] = TextureImporter::load_texture_from_file("Assets/Icons/ThreeDots.png").value();
    editor_icons[Search] = TextureImporter::load_texture_from_file("Assets/Icons/Search.png").value();

    editor_icons[Error] = TextureImporter::load_texture_from_file("Assets/Icons/ErrorIcon.png").value();
    editor_icons[Warning] = TextureImporter::load_texture_from_file("Assets/Icons/WarningIcon.png").value();
    editor_icons[Info] = TextureImporter::load_texture_from_file("Assets/Icons/InfoIcon.png").value();
    editor_icons[CogWheel] = TextureImporter::load_texture_from_file("Assets/Icons/CogWheel.png").value();
    editor_icons[Entity] = TextureImporter::load_texture_from_file("Assets/Icons/Entity.png").value();

    editor_icons[Play] = TextureImporter::load_texture_from_file("Assets/Icons/PlayButton.png").value();
    editor_icons[Stop] = TextureImporter::load_texture_from_file("Assets/Icons/StopButton.png").value();
    editor_icons[Pause] = TextureImporter::load_texture_from_file("Assets/Icons/PauseButton.png").value();
    editor_icons[StepFrame] = TextureImporter::load_texture_from_file("Assets/Icons/StepFrame.png").value();
}

EditorStyle& EditorStyle::get_style()
{
    return g_Style;
}
