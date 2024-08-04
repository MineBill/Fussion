#pragma once
#include "Math.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/Log/Formatters.h"

namespace Fussion {
    struct Color {
        struct HSL {
            f32 H{}, S{}, L{}, A{};

            [[nodiscard]]
            static constexpr HSL FromRGB(Color color)
            {
                HSL hsl;
                hsl.A = color.A;
                auto max = Math::Max(color.R, color.G, color.B);
                auto min = Math::Min(color.R, color.G, color.B);

                hsl.L = (min + max) / 2.0f;

                if (max == min) {
                    return hsl;
                }

                auto d = max - min;

                hsl.S = (hsl.L > 0.5f) ? d / (2.0f - max - min) : d / (max + min);

                if (color.R > color.G && color.R > color.B) {
                    hsl.H = (color.G - color.B) / d + (color.G < color.B ? 6.0f : 0.0f);
                } else if (color.G > color.B) {
                    hsl.H = (color.B - color.R) / +2.0f;
                } else {
                    hsl.H = (color.R - color.G) / d + 4.0f;
                }

                hsl.H /= 6.0f;
                return hsl;
            }
        };

#if OS_WINDOWS
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
        union {
            struct {
                f32 R, G, B, A;
            };

            f32 Raw[4]{};
        };
#if OS_WINDOWS
#pragma warning(pop)
#endif

        constexpr Color(): R(0), G(0), B(0), A(1) {}
        constexpr Color(f32 r, f32 g, f32 b, f32 a): R(r), G(g), B(b), A(a) {}
        Color(Vector4 v): R(v.X), G(v.Y), B(v.Z), A(v.W) {}

        [[nodiscard]]
        static constexpr Color FromHex(u32 hex)
        {
            return {
                CAST(f32, hex >> 24) / 256.f,
                CAST(f32, hex >> 16 & 0x00FF) / 256.f,
                CAST(f32, hex >> 8 & 0x0000FF) / 256.f,
                CAST(f32, hex & 0x000000FF) / 256.f
            };
        }

        static constexpr Color FromRGBA(u8 r, u8 g, u8 b, u8 a = 255)
        {
            return {
                CAST(f32, r) / 255.f,
                CAST(f32, g) / 255.f,
                CAST(f32, b) / 255.f,
                CAST(f32, a) / 255.f
            };
        }

        [[nodiscard]]
        static constexpr Color FromHSL(HSL hsl)
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
                    return p + (q - p) * 6.f * t;
                }
                if (t < 1. / 2.f) {
                    return q;
                }
                if (t < 2. / 3.f) {
                    return p + (q - p) * (2.f / 3.f - t) * 6.f;
                }
                return p;
            };

            if (hsl.S == 0) {
                return { hsl.L, hsl.L, hsl.L, hsl.A };
            } else {
                auto q = hsl.L < 0.5f ? hsl.L * (1.f + hsl.S) : hsl.L + hsl.S - hsl.L * hsl.S;
                auto p = 2 * hsl.L - q;
                color.R = HueToRGB(p, q, hsl.H + 1.f / 3.f);
                color.G = HueToRGB(p, q, hsl.H);
                color.B = HueToRGB(p, q, hsl.H - 1.f / 3.f);
                color.A = hsl.A;
            }
            return color;
        }

        [[nodiscard]]
        constexpr Color Lighten(f32 percent) const
        {
            auto hsl = ToHSL();
            hsl.L += percent;
            return FromHSL(hsl);
        }

        [[nodiscard]]
        constexpr Color Darken(f32 percent) const
        {
            auto hsl = ToHSL();
            hsl.L -= percent;
            return FromHSL(hsl);
        }

        [[nodiscard]]
        constexpr HSL ToHSL() const
        {
            return HSL::FromRGB(*this);
        }

        u32 ToABGR();

        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Magenta;
        static const Color Cyan;
        static const Color Black;
        static const Color Purple;
        static const Color Orange;
        static const Color Pink;
        static const Color Turquoise;
        static const Color Lime;
        static const Color Gray;
        static const Color Brown;
        static const Color Maroon;
        static const Color Teal;
        static const Color Olive;
        static const Color Navy;
        static const Color Coral;
        static const Color Rose;
        static const Color SkyBlue;
        static const Color ForestGreen;
        static const Color DarkGoldenRod;
        static const Color Indigo;
        static const Color Transparent;

    };

    inline constexpr Color Color::White{ 1, 1, 1, 1 };
    inline constexpr Color Color::Red{ 1, 0, 0, 1 };
    inline constexpr Color Color::Green{ 0, 1, 0, 1 };
    inline constexpr Color Color::Blue{ 0, 0, 1, 1 };
    inline constexpr Color Color::Yellow{ 1, 1, 0, 1 };
    inline constexpr Color Color::Magenta{ 1, 0, 1, 1 };
    inline constexpr Color Color::Cyan{ 0, 1, 1, 1 };
    inline constexpr Color Color::Black{ 0, 0, 0, 1 };
    inline constexpr Color Color::Purple{ 0.5f, 0, 0.5f, 1 };
    inline constexpr Color Color::Orange{ 1, 0.5f, 0, 1 };
    inline constexpr Color Color::Pink{ 1, 0.75f, 0.8f, 1 };
    inline constexpr Color Color::Turquoise{ 0, 0.8f, 0.8f, 1 };
    inline constexpr Color Color::Lime{ 0.75f, 1, 0, 1 };
    inline constexpr Color Color::Gray{ 0.5f, 0.5f, 0.5f, 1 };
    inline constexpr Color Color::Brown{ 0.6f, 0.4f, 0.2f, 1 };
    inline constexpr Color Color::Maroon{ 0.5f, 0, 0, 1 };
    inline constexpr Color Color::Teal{ 0, 0.5f, 0.5f, 1 };
    inline constexpr Color Color::Olive{ 0.5f, 0.5f, 0, 1 };
    inline constexpr Color Color::Navy{ 0, 0, 0.5f, 1 };
    inline constexpr Color Color::Coral{ 1, 0.5f, 0.3f, 1 };
    inline constexpr Color Color::Rose{ 1, 0.2f, 0.5f, 1 };
    inline constexpr Color Color::SkyBlue{ 0.5, 0.75, 1, 1 };
    inline constexpr Color Color::ForestGreen{ 0.2f, 0.5, 0.2f, 1 };
    inline constexpr Color Color::DarkGoldenRod{ 0.72f, 0.52f, 0.04f, 1 };
    inline constexpr Color Color::Indigo{ 0.29f, 0, 0.51f, 1 };
    inline constexpr Color Color::Transparent{ 0, 0, 0, 0 };
}


#if FSN_MATH_USE_GLOBALLY
using Fussion::Color;
using HSL = Fussion::Color::HSL;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Color, "Color(R: {}, G: {}, B: {}, A: {})", v.R, v.G, v.B, v.A)

FSN_MAKE_FORMATTABLE(Fussion::Color::HSL, "HSL(H: {}, S: {}, L: {}, A: {})", v.H, v.S, v.L, v.A)
