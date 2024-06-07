#pragma once
#include <string>
#include <unordered_map>

struct ImFont;

struct EditorFonts
{
    ImFont* RegularNormal{nullptr};
    ImFont* RegularSmall{nullptr};

    ImFont* Bold{nullptr};
    ImFont* BoldSmall{nullptr};
};

struct EditorStyle
{
    static EditorStyle Default();

    EditorFonts Fonts;
};
