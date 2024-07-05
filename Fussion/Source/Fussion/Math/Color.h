#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion {
struct Color {
    union {
        struct {
            f32 R, G, B, A;
        };

        f32 Raw[4]{};
    };

    Color() = default;
    Color(f32 r, f32 g, f32 b, f32 a): R(r), G(g), B(b), A(a) {}
};

}
