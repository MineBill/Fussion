#include "EditorPCH.h"
#include "EditorStyle.h"

#include <Fussion/Util/TextureImporter.h>

#include <filesystem>

using namespace Fussion;

EditorStyle g_Style;

void EditorStyle::Initialize()
{
    ButtonStyles[ButtonStyleGeneric] = ButtonStyle::Default();
    ButtonStyles[ButtonStyleDisabled] = ButtonStyle::Default();

    {
        auto style = ButtonStyle::Default();
        style.Padding = Vector2::Zero;
        style.Border = false;
        style.NormalColor = Color::Transparent;
        style.HoverColor = Color::Black;
        style.HoverColor.a = 0.2f;

        style.PressedColor = Color::White;
        style.PressedColor.a = 0.2f;

        ButtonStyles[ButtonStyleImageButton] = style;
    }

    {
        auto style = ButtonStyle::Default();
        style.Border = false;
        style.Rounding = 3.0f;

        style.SetButtonColor(Color::FromHex(ACCENT_COLOR));

        ButtonStyles[ButtonStyleViewportButton] = style;
    }

    {
        auto style = ButtonStyle::Default();
        style.Border = false;
        style.Rounding = 3.0f;
        style.Font = EditorFont::RegularBig;

        style.SetButtonColor(Color::FromHex(ACCENT_COLOR));

        ButtonStyles[ButtonStyleProjectCreator] = style;
    }

    {
        auto style = ButtonStyle::Default();
        style.Border = false;
        style.Rounding = 3.0f;
        style.Font = EditorFont::RegularNormal;

        style.SetButtonColor(Color::FromHex(ACCENT_COLOR));

        ButtonStyles[ButtonStyleProjectCreatorSmall] = style;
    }

    {
        auto style = WindowStyle();
        style.Padding = { 3, 3 };
        WindowStyles[WindowStyleGeneric] = style;
    }

    {
        auto style = WindowStyle();
        style.Border = false;
        style.Padding = { 10, 10 };
        style.Rounding = 0.0;
        WindowStyles[WindowStyleCreator] = style;
    }

    {
        auto style = WindowStyle();
        style.Border = true;
        style.Padding = { 5, 5 };
        WindowStyles[WindowStyleAssetPreview] = style;
    }

    using enum EditorIcon;
    EditorIcons[Folder] = TextureImporter::LoadTextureFromFile("Assets/Icons/Folder.png").Unwrap();
    EditorIcons[FolderBack] = TextureImporter::LoadTextureFromFile("Assets/Icons/FolderBack.png").Unwrap();
    EditorIcons[GenericAsset] = TextureImporter::LoadTextureFromFile("Assets/Icons/GenericAsset.png").Unwrap();
    EditorIcons[Scene] = TextureImporter::LoadTextureFromFile("Assets/Icons/Scene.png").Unwrap();
    EditorIcons[Script] = TextureImporter::LoadTextureFromFile("Assets/Icons/Script.png").Unwrap();
    EditorIcons[PbrMaterial] = TextureImporter::LoadTextureFromFile("Assets/Icons/PbrMaterial.png").Unwrap();
    EditorIcons[Dots] = TextureImporter::LoadTextureFromFile("Assets/Icons/ThreeDots.png").Unwrap();
    EditorIcons[Search] = TextureImporter::LoadTextureFromFile("Assets/Icons/Search.png").Unwrap();

    EditorIcons[Error] = TextureImporter::LoadTextureFromFile("Assets/Icons/ErrorIcon.png").Unwrap();
    EditorIcons[Warning] = TextureImporter::LoadTextureFromFile("Assets/Icons/WarningIcon.png").Unwrap();
    EditorIcons[Info] = TextureImporter::LoadTextureFromFile("Assets/Icons/InfoIcon.png").Unwrap();
    EditorIcons[CogWheel] = TextureImporter::LoadTextureFromFile("Assets/Icons/CogWheel.png").Unwrap();
    EditorIcons[Entity] = TextureImporter::LoadTextureFromFile("Assets/Icons/Entity.png").Unwrap();

    EditorIcons[Play] = TextureImporter::LoadTextureFromFile("Assets/Icons/PlayButton.png").Unwrap();
    EditorIcons[Stop] = TextureImporter::LoadTextureFromFile("Assets/Icons/StopButton.png").Unwrap();
    EditorIcons[Pause] = TextureImporter::LoadTextureFromFile("Assets/Icons/PauseButton.png").Unwrap();
    EditorIcons[StepFrame] = TextureImporter::LoadTextureFromFile("Assets/Icons/StepFrame.png").Unwrap();
}

EditorStyle& EditorStyle::Style()
{
    return g_Style;
}
