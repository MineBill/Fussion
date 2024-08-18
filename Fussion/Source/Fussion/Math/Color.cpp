#include "FussionPCH.h"
#include "Color.h"

namespace Fussion {



    u32 Color::ToABGR()
    {
        auto r = CAST(u8, R * 255);
        auto g = CAST(u8, G * 255);
        auto b = CAST(u8, B * 255);
        auto a = CAST(u8, A * 255);
        return (CAST(u32, a) << 24) | (CAST(u32, b) << 16) | (CAST(u32, g) << 8) | CAST(u32, r);
    }
}
