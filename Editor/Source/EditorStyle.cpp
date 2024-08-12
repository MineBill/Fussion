#include "EditorStyle.h"

#include <Fussion/Util/TextureImporter.h>

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
        style.HoverColor.A = 0.2f;
        
        style.PressedColor = Color::White;
        style.PressedColor.A = 0.2f;
        
        ButtonStyles[ButtonStyleImageButton] = style;
    }
    
    {
        auto style = ButtonStyle::Default();
        style.Border = false;
        style.Rounding = 3.0f;
        
        style.SetButtonColor(Color::FromHex(AccentColor));
        
        ButtonStyles[ButtonStyleViewportButton] = style;
    }
    
    {
        auto style = ButtonStyle::Default();
        style.Border = false;
        style.Rounding = 3.0f;
        style.Font = EditorFont::RegularBig;
        
        style.SetButtonColor(Color::FromHex(AccentColor));
        
        ButtonStyles[ButtonStyleProjectCreator] = style;
    }
    
    {
        auto style = ButtonStyle::Default();
        style.Border = false;
        style.Rounding = 3.0f;
        style.Font = EditorFont::RegularNormal;
        
        style.SetButtonColor(Color::FromHex(AccentColor));
        
        ButtonStyles[ButtonStyleProjectCreatorSmall] = style;
    }
    
    {
        auto style = WindowStyle();
        style.Border = false;
        style.Padding = { 10, 10 };
        style.Rounding = 0.0;
        WindowStyles[WindowStyleCreator] = style;
    }
    
    using enum EditorIcon;
    EditorIcons[Folder] = TextureImporter::LoadTextureFromFile("Assets/Icons/Folder.png");
    EditorIcons[FolderBack] = TextureImporter::LoadTextureFromFile("Assets/Icons/FolderBack.png");
    EditorIcons[GenericAsset] = TextureImporter::LoadTextureFromFile("Assets/Icons/GenericAsset.png");
    EditorIcons[Scene] = TextureImporter::LoadTextureFromFile("Assets/Icons/Scene.png");
    EditorIcons[Script] = TextureImporter::LoadTextureFromFile("Assets/Icons/Script.png");
    EditorIcons[PbrMaterial] = TextureImporter::LoadTextureFromFile("Assets/Icons/PbrMaterial.png");
    EditorIcons[Dots] = TextureImporter::LoadTextureFromFile("Assets/Icons/ThreeDots.png");
    EditorIcons[Search] = TextureImporter::LoadTextureFromFile("Assets/Icons/Search.png");
    
    EditorIcons[Error] = TextureImporter::LoadTextureFromFile("Assets/Icons/ErrorIcon.png");
    EditorIcons[Warning] = TextureImporter::LoadTextureFromFile("Assets/Icons/WarningIcon.png");
    EditorIcons[Info] = TextureImporter::LoadTextureFromFile("Assets/Icons/InfoIcon.png");
    EditorIcons[CogWheel] = TextureImporter::LoadTextureFromFile("Assets/Icons/CogWheel.png");
    
    EditorIcons[Play] = TextureImporter::LoadTextureFromFile("Assets/Icons/PlayButton.png");
    EditorIcons[Stop] = TextureImporter::LoadTextureFromFile("Assets/Icons/StopButton.png");
    EditorIcons[Pause] = TextureImporter::LoadTextureFromFile("Assets/Icons/PauseButton.png");
    EditorIcons[StepFrame] = TextureImporter::LoadTextureFromFile("Assets/Icons/StepFrame.png");
}

EditorStyle& EditorStyle::GetStyle()
{
    return g_Style;
}
