#pragma once
#include "Math.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector4.h"

namespace Fussion {
struct Color {
    struct HSL {
        f32 H{}, S{}, L{}, A{};

        [[nodiscard]]
        static HSL FromRGB(Color color)
        {
            HSL hsl;
            hsl.A = color.A;
            auto max = Math::Max(color.R, color.G, color.B);
            auto min = Math::Min(color.R, color.G, color.B);

            hsl.L = (min + max) / 2.0;

            if (max == min) {
                return hsl;
            }

            auto d = max - min;

            hsl.S = (hsl.L > 0.5) ? d / (2.0 - max - min) : d / (max + min);

            if (color.R > color.G && color.R > color.B) {
                hsl.H = (color.G - color.B) / d + (color.G < color.B ? 6.0 : 0.0);
            } else if (color.G > color.B) {
                hsl.H = (color.B - color.R) / +2.0f;
            } else {
                hsl.H = (color.R - color.G) / d + 4.0;
            }

            hsl.H /= 6.0f;
            return hsl;
        }
    };

    union {
        struct {
            f32 R, G, B, A;
        };

        f32 Raw[4]{};
    };

    Color() = default;
    Color(f32 r, f32 g, f32 b, f32 a): R(r), G(g), B(b), A(a) {}
    Color(Vector4 v): R(v.X), G(v.Y), B(v.Z), A(v.W) {}

    [[nodiscard]]
    static Color FromHex(s32 hex)
    {
        return {
            CAST(f32, hex >> 24) / 256.f,
            CAST(f32, hex >> 16 & 0x00FF) / 256.f,
            CAST(f32, hex >> 8 & 0x0000FF) / 256.f,
            CAST(f32, hex & 0x000000FF) / 256.f
        };
    }

    [[nodiscard]]
    static Color FromHSL(HSL hsl)
    {
        Color color;

        auto HueToRGB = [](f32 p, f32 q, f32 t) -> f32 {
            if (t < 0) {
                t += 1;
            }
            if (t > 1) {
                t -= 1.f;
            }

            if (t < 1. / 6.f) {
                return p + (q - p) * 6. * t;
            }
            if (t < 1. / 2.f) {
                return q;
            }
            if (t < 2. / 3.f) {
                return p + (q - p) * (2. / 3. - t) * 6.;
            }
            return p;
        };

        if (hsl.S == 0) {
            return { hsl.L, hsl.L, hsl.L, hsl.A };
        } else {
            auto q = hsl.L < 0.5 ? hsl.L * (1 + hsl.S) : hsl.L + hsl.S - hsl.L * hsl.S;
            auto p = 2 * hsl.L - q;
            color.R = HueToRGB(p, q, hsl.H + 1. / 3.);
            color.G = HueToRGB(p, q, hsl.H);
            color.B = HueToRGB(p, q, hsl.H - 1. / 3.);
            color.A = hsl.A;
        }
        return color;
    }

    [[nodiscard]]
    Color Lighten(f32 percent) const
    {
        auto hsl = ToHSL();
        hsl.L += percent;
        return FromHSL(hsl);
    }

    [[nodiscard]]
    Color Darken(f32 percent) const
    {
        auto hsl = ToHSL();
        hsl.L -= percent;
        return FromHSL(hsl);
    }

    [[nodiscard]]
    HSL ToHSL() const
    {
        return HSL::FromRGB(*this);
    }

    static Color White;
    static Color Red;
    static Color Green;
    static Color Blue;
    static Color Yellow;
    static Color Magenta;
    static Color Cyan;
    static Color Black;
    static Color Purple;
    static Color Orange;
    static Color Pink;
    static Color Turquoise;
    static Color Lime;
    static Color Gray;
    static Color Brown;
    static Color Maroon;
    static Color Teal;
    static Color Olive;
    static Color Navy;
    static Color Coral;
    static Color Rose;
    static Color SkyBlue;
    static Color ForestGreen;
    static Color DarkGoldenRod;
    static Color Indigo;
};
}

#if FSN_MATH_USE_GLOBALLY
using Fussion::Color;
using HSL = Fussion::Color::HSL;
#endif
