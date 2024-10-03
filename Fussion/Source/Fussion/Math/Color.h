#pragma once
#include <Fussion/Core/Types.h>
#include <Fussion/Log/Formatters.h>
#include <Fussion/Math/Math.h>
#include <Fussion/Math/Vector4.h>

namespace Fussion {
    struct Color {
        struct HSL {
            f32 h {}, s {}, l {}, a {};

            [[nodiscard]]
            static constexpr HSL FromRGB(Color color)
            {
                HSL hsl;
                hsl.a = color.a;
                auto max = Math::Max(color.r, color.g, color.b);
                auto min = Math::Min(color.r, color.g, color.b);

                hsl.l = (min + max) / 2.0f;

                if (max == min) {
                    return hsl;
                }

                auto d = max - min;

                hsl.s = (hsl.l > 0.5f) ? d / (2.0f - max - min) : d / (max + min);

                if (color.r > color.g && color.r > color.b) {
                    hsl.h = (color.g - color.b) / d + (color.g < color.b ? 6.0f : 0.0f);
                } else if (color.g > color.b) {
                    hsl.h = (color.b - color.r) / +2.0f;
                } else {
                    hsl.h = (color.r - color.g) / d + 4.0f;
                }

                hsl.h /= 6.0f;
                return hsl;
            }
        };

#if OS_WINDOWS
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
        union {
            struct {
                f32 r, g, b, a;
            };

            f32 raw[4] {};
        };
#if OS_WINDOWS
#    pragma warning(pop)
#endif

        constexpr Color()
            : r(0)
            , g(0)
            , b(0)
            , a(1)
        { }
        constexpr Color(f32 r, f32 g, f32 b, f32 a)
            : r(r)
            , g(g)
            , b(b)
            , a(a)
        { }
        Color(Vector4 v)
            : r(v.x)
            , g(v.y)
            , b(v.z)
            , a(v.w)
        { }

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

        [[nodiscard]]
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

            auto hue_to_rgb = [](f32 p, f32 q, f32 t) -> f32 {
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

            if (hsl.s == 0) {
                return { hsl.l, hsl.l, hsl.l, hsl.a };
            }
            auto q = hsl.l < 0.5f ? hsl.l * (1.f + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
            auto p = 2 * hsl.l - q;
            color.r = hue_to_rgb(p, q, hsl.h + 1.f / 3.f);
            color.g = hue_to_rgb(p, q, hsl.h);
            color.b = hue_to_rgb(p, q, hsl.h - 1.f / 3.f);
            color.a = hsl.a;
            return color;
        }

        [[nodiscard]]
        constexpr Color Lighten(f32 percent) const
        {
            auto hsl = ToHSL();
            hsl.l += percent;
            return FromHSL(hsl);
        }

        [[nodiscard]]
        constexpr Color Darken(f32 percent) const
        {
            auto hsl = ToHSL();
            hsl.l -= percent;
            return FromHSL(hsl);
        }

        [[nodiscard]]
        constexpr HSL ToHSL() const
        {
            return HSL::FromRGB(*this);
        }

        u32 ToABGR() const;

        static Color const White;
        static Color const Red;
        static Color const Green;
        static Color const Blue;
        static Color const Yellow;
        static Color const Magenta;
        static Color const Cyan;
        static Color const Black;
        static Color const Purple;
        static Color const Orange;
        static Color const Pink;
        static Color const Turquoise;
        static Color const Lime;
        static Color const Gray;
        static Color const Brown;
        static Color const Maroon;
        static Color const Teal;
        static Color const Olive;
        static Color const Navy;
        static Color const Coral;
        static Color const Rose;
        static Color const SkyBlue;
        static Color const ForestGreen;
        static Color const DarkGoldenRod;
        static Color const Indigo;
        static Color const Transparent;
    };

    inline constexpr Color Color::White { 1, 1, 1, 1 };
    inline constexpr Color Color::Red { 1, 0, 0, 1 };
    inline constexpr Color Color::Green { 0, 1, 0, 1 };
    inline constexpr Color Color::Blue { 0, 0, 1, 1 };
    inline constexpr Color Color::Yellow { 1, 1, 0, 1 };
    inline constexpr Color Color::Magenta { 1, 0, 1, 1 };
    inline constexpr Color Color::Cyan { 0, 1, 1, 1 };
    inline constexpr Color Color::Black { 0, 0, 0, 1 };
    inline constexpr Color Color::Purple { 0.5f, 0, 0.5f, 1 };
    inline constexpr Color Color::Orange { 1, 0.5f, 0, 1 };
    inline constexpr Color Color::Pink { 1, 0.75f, 0.8f, 1 };
    inline constexpr Color Color::Turquoise { 0, 0.8f, 0.8f, 1 };
    inline constexpr Color Color::Lime { 0.75f, 1, 0, 1 };
    inline constexpr Color Color::Gray { 0.5f, 0.5f, 0.5f, 1 };
    inline constexpr Color Color::Brown { 0.6f, 0.4f, 0.2f, 1 };
    inline constexpr Color Color::Maroon { 0.5f, 0, 0, 1 };
    inline constexpr Color Color::Teal { 0, 0.5f, 0.5f, 1 };
    inline constexpr Color Color::Olive { 0.5f, 0.5f, 0, 1 };
    inline constexpr Color Color::Navy { 0, 0, 0.5f, 1 };
    inline constexpr Color Color::Coral { 1, 0.5f, 0.3f, 1 };
    inline constexpr Color Color::Rose { 1, 0.2f, 0.5f, 1 };
    inline constexpr Color Color::SkyBlue { 0.5, 0.75, 1, 1 };
    inline constexpr Color Color::ForestGreen { 0.2f, 0.5, 0.2f, 1 };
    inline constexpr Color Color::DarkGoldenRod { 0.72f, 0.52f, 0.04f, 1 };
    inline constexpr Color Color::Indigo { 0.29f, 0, 0.51f, 1 };
    inline constexpr Color Color::Transparent { 0, 0, 0, 0 };
}

#if FSN_MATH_USE_GLOBALLY
using Fussion::Color;
using HSL = Fussion::Color::HSL;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Color, "Color(R: {}, G: {}, B: {}, A: {})", v.r, v.g, v.b, v.a)

FSN_MAKE_FORMATTABLE(Fussion::Color::HSL, "HSL(H: {}, S: {}, L: {}, A: {})", v.h, v.s, v.l, v.a)
