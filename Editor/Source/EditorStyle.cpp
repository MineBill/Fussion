#include "EditorStyle.h"

EditorStyle EditorStyle::Default()
{
    EditorStyle style;
    style.ButtonStyles[ButtonStyleGeneric] = ButtonStyle::Default();
    style.ButtonStyles[ButtonStyleDisabled] = ButtonStyle::Default();

    return style;
}
