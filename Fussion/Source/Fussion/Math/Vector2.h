#pragma once
#include <Fussion/Core/Concepts.h>
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Log/Formatters.h"

namespace Fussion {
    struct Vector2 {

#if USE_VECTOR_F64
    using Real = f64;
#else
        using Real = f32;
#endif

#if OS_WINDOWS
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
        union {
            struct {
                Real x, y;
            };

            Real raw[2];
        };
#if OS_WINDOWS
#pragma warning(pop)
#endif

        Vector2() = default;
        constexpr Vector2(f32 x, f32 y): x(x), y(y) {}
        constexpr Vector2(f64 x, f64 y): x(CAST(Real, x)), y(CAST(Real, y)) {}
        constexpr Vector2(ScalarType auto x, ScalarType auto y): x(CAST(Real, x)), y(CAST(Real, y)) {}
        constexpr Vector2(Vector2 const& other): x(other.x), y(other.y) {}

        [[nodiscard]]
        Real length() const;

        [[nodiscard]]
        Real length_squared() const;

        [[nodiscard]]
        Real distance_to(Vector2 const& other) const;

        [[nodiscard]]
        Real distance_to_squared(Vector2 const& other) const;

        [[nodiscard]]
        bool is_zero() const;

        [[nodiscard]]
        f32 aspect() const;

        constexpr Real& operator[](std::size_t i)
        {
            return raw[i];
        }

        constexpr Real operator[](std::size_t i) const
        {
            return raw[i];
        }

        constexpr Vector2& operator=(Vector2 const& other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }

        constexpr Vector2& operator+=(Vector2 const& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector2 operator+(Vector2 const& right) const;
        Vector2 operator-(Vector2 const& right) const;
        Vector2 operator*(Vector2 const& right) const;
        Vector2 operator/(Vector2 const& right) const;

        constexpr Vector2 operator+(ScalarType auto scalar) const
        {
            return { x + CAST(Real, scalar), y + CAST(Real, scalar) };
        }

        constexpr Vector2 operator-(ScalarType auto scalar) const
        {
            return { x - CAST(Real, scalar), y - CAST(Real, scalar) };
        }

        constexpr Vector2 operator*(ScalarType auto scalar) const
        {
            return { x * CAST(Real, scalar), y * CAST(Real, scalar) };
        }

        constexpr Vector2 operator/(ScalarType auto scalar) const
        {
            return { x / CAST(Real, scalar), y / CAST(Real, scalar) };
        }

        constexpr Vector2& operator+=(ScalarType auto scalar)
        {
            *this = *this + CAST(Real, scalar);
            return *this;
        }

        constexpr Vector2& operator-=(ScalarType auto scalar)
        {
            *this = *this - CAST(Real, scalar);
            return *this;
        }

        constexpr Vector2& operator*=(ScalarType auto scalar)
        {
            *this = *this * CAST(Real, scalar);
            return *this;
        }

        constexpr Vector2& operator/=(ScalarType auto scalar)
        {
            *this = *this / CAST(Real, scalar);
            return *this;
        }

        Vector2(glm::vec2 v): x(v.x), y(v.y) {}

        operator glm::vec2() const
        {
            return { x, y };
        }

        static Vector2 const Zero;
        static Vector2 const One;
        static Vector2 const Up;
        static Vector2 const Down;
        static Vector2 const Left;
        static Vector2 const Right;
    };

    bool operator==(Vector2 const& lhs, Vector2 const& rhs);
    bool operator!=(Vector2 const& lhs, Vector2 const& rhs);

    inline constexpr Vector2 Vector2::Zero{ 0, 0 };
    inline constexpr Vector2 Vector2::One{ 1, 1 };
    inline constexpr Vector2 Vector2::Up{ 0, 1 };
    inline constexpr Vector2 Vector2::Down{ 0, -1 };
    inline constexpr Vector2 Vector2::Left{ 1, 0 };
    inline constexpr Vector2 Vector2::Right{ -1, 0 };
}

#if defined(FSN_MATH_USE_GLOBALLY)
using Fussion::Vector2;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Vector2, "Vector2({:.2f}, {:.2f})", v.x, v.y)
