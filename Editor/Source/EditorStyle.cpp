#include "EditorStyle.h"

void ButtonStyle::SetButtonColor(Color color)
{
    NormalColor = color;
    HoverColor = NormalColor.Darken(0.1f);
    PressedColor = NormalColor.Lighten(0.1f);
}

void EditorStyle::Init()
{
    ButtonStyles[ButtonStyleGeneric] = ButtonStyle::Default();
    ButtonStyles[ButtonStyleDisabled] = ButtonStyle::Default();

    {
        auto style = ButtonStyle::Default();
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
        style.Rounding = 5.0f;

        style.SetButtonColor(Color::FromHex(AccentColor));

        ButtonStyles[ButtonStyleViewportButton] = style;
    }
}
