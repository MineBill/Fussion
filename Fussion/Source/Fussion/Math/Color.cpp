#include "e5pch.h"
#include "Color.h"

namespace Fussion {
Color Color::White{ 1, 1, 1, 1 };
Color Color::Red{ 1, 0, 0, 1 };
Color Color::Green{ 0, 1, 0, 1 };
Color Color::Blue{ 0, 0, 1, 1 };
Color Color::Yellow{ 1, 1, 0, 1 };
Color Color::Magenta{ 1, 0, 1, 1 };
Color Color::Cyan{ 0, 1, 1, 1 };
Color Color::Black{ 0, 0, 0, 1 };
Color Color::Purple{ 0.5f, 0, 0.5f, 1 };
Color Color::Orange{ 1, 0.5f, 0, 1 };
Color Color::Pink{ 1, 0.75f, 0.8f, 1 };
Color Color::Turquoise{ 0, 0.8f, 0.8f, 1 };
Color Color::Lime{ 0.75f, 1, 0, 1 };
Color Color::Gray{ 0.5f, 0.5f, 0.5f, 1 };
Color Color::Brown{ 0.6f, 0.4f, 0.2f, 1 };
Color Color::Maroon{ 0.5f, 0, 0, 1 };
Color Color::Teal{ 0, 0.5f, 0.5f, 1 };
Color Color::Olive{ 0.5f, 0.5f, 0, 1 };
Color Color::Navy{ 0, 0, 0.5f, 1 };
Color Color::Coral{ 1, 0.5f, 0.3f, 1 };
Color Color::Rose{ 1, 0.2f, 0.5f, 1 };
Color Color::SkyBlue{ 0.5, 0.75, 1, 1 };
Color Color::ForestGreen{ 0.2f, 0.5, 0.2f, 1 };
Color Color::DarkGoldenRod{ 0.72f, 0.52f, 0.04f, 1 };
Color Color::Indigo{ 0.29f, 0, 0.51f, 1 };
Color Color::Transparent{ 0, 0, 0, 0 };

u32 Color::ToABGR()
{
    auto r = CAST(u8, R * 255);
    auto g = CAST(u8, G * 255);
    auto b = CAST(u8, B * 255);
    auto a = CAST(u8, A * 255);
    return (CAST(u32, a) << 24) | (CAST(u32, b) << 16) | (CAST(u32, g) << 8) | CAST(u32, r);
}
}
