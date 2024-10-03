#include "FussionPCH.h"
#include "Color.h"

namespace Fussion {
    u32 Color::ToABGR() const
    {
        auto r8 = CAST(u8, r * 255);
        auto g8 = CAST(u8, g * 255);
        auto b8 = CAST(u8, b * 255);
        auto a8 = CAST(u8, a * 255);
        return (CAST(u32, a8) << 24) | (CAST(u32, b8) << 16) | (CAST(u32, g8) << 8) | CAST(u32, r8);
    }
}
