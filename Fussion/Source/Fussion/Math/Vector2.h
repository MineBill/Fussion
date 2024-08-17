#pragma once
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
                Real X, Y;
            };

            Real Raw[2];
        };
#if OS_WINDOWS
#pragma warning(pop)
#endif

        Vector2() = default;
        constexpr Vector2(f32 x, f32 y): X(x), Y(y) {}
        constexpr Vector2(f64 x, f64 y): X(CAST(Real, x)), Y(CAST(Real, y)) {}
        constexpr Vector2(ScalarType auto x, ScalarType auto y): X(CAST(Real, x)), Y(CAST(Real, y)) {}
        constexpr Vector2(Vector2 const& other): X(other.X), Y(other.Y) {}

        [[nodiscard]]
        Real Length() const;

        [[nodiscard]]
        Real LengthSquared() const;

        [[nodiscard]]
        Real DistanceTo(Vector2 const& other) const;

        [[nodiscard]]
        Real DistanceToSquared(Vector2 const& other) const;

        [[nodiscard]]
        bool IsZero() const;

        [[nodiscard]]
        f32 Aspect() const;

        constexpr Real& operator[](std::size_t i)
        {
            return Raw[i];
        }

        constexpr Real operator[](std::size_t i) const
        {
            return Raw[i];
        }

        constexpr Vector2& operator=(Vector2 const& other)
        {
            X = other.X;
            Y = other.Y;
            return *this;
        }

        constexpr Vector2& operator+=(Vector2 const& other)
        {
            X += other.X;
            Y += other.Y;
            return *this;
        }

        Vector2 operator+(Vector2 const& right) const;
        Vector2 operator-(Vector2 const& right) const;
        Vector2 operator*(Vector2 const& right) const;
        Vector2 operator/(Vector2 const& right) const;

        constexpr Vector2 operator+(ScalarType auto scalar) const
        {
            return { X + CAST(Real, scalar), Y + CAST(Real, scalar) };
        }

        constexpr Vector2 operator-(ScalarType auto scalar) const
        {
            return { X - CAST(Real, scalar), Y - CAST(Real, scalar) };
        }

        constexpr Vector2 operator*(ScalarType auto scalar) const
        {
            return { X * CAST(Real, scalar), Y * CAST(Real, scalar) };
        }

        constexpr Vector2 operator/(ScalarType auto scalar) const
        {
            return { X / CAST(Real, scalar), Y / CAST(Real, scalar) };
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

        Vector2(glm::vec2 v): X(v.x), Y(v.y) {}

        operator glm::vec2() const
        {
            return { X, Y };
        }

        static const Vector2 Zero;
        static const Vector2 One;
        static const Vector2 Up;
        static const Vector2 Down;
        static const Vector2 Left;
        static const Vector2 Right;
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

FSN_MAKE_FORMATTABLE(Fussion::Vector2, "Vector2({:.2f}, {:.2f})", v.X, v.Y)
