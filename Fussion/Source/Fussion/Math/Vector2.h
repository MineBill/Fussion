#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Log/Formatters.h"
#include <cmath>

namespace Fussion {
struct Vector2 {
#if USE_VECTOR_F64
    using Real = f64;
#else
    using Real = f32;
#endif

    union {
        struct {
            Real X, Y;
        };

        Real Raw[2];
    };

    Vector2() = default;
    Vector2(ScalarType auto x, ScalarType auto y): X(x), Y(y) {}

    // Vector2(Vector2 const& v): X(v.X), Y(v.Y) {}

    [[nodiscard]] Real Length() const;

    [[nodiscard]] Real LengthSquared() const;

    [[nodiscard]] Real DistanceTo(Vector2 const& other) const;

    [[nodiscard]] Real DistanceToSquared(Vector2 const& other) const;

    Real& operator[](std::size_t i)
    {
        return Raw[i];
    }

    Real operator[](std::size_t i) const
    {
        return Raw[i];
    }

    Vector2 operator+(Vector2 const& right) const;
    Vector2 operator-(Vector2 const& right) const;
    Vector2 operator*(Vector2 const& right) const;
    Vector2 operator/(Vector2 const& right) const;

    Vector2 operator+(ScalarType auto scalar) const
    {
        return { X + CAST(Real, scalar), Y + CAST(Real, scalar) };
    }

    Vector2 operator-(ScalarType auto scalar) const
    {
        return { X - CAST(Real, scalar), Y - CAST(Real, scalar) };
    }

    Vector2 operator*(ScalarType auto scalar) const
    {
        return { X * CAST(Real, scalar), Y * CAST(Real, scalar) };
    }

    Vector2 operator/(ScalarType auto scalar) const
    {
        return { X / CAST(Real, scalar), Y / CAST(Real, scalar) };
    }

    Vector2 operator+=(ScalarType auto scalar)
    {
        *this = *this + CAST(Real, scalar);
        return *this;
    }

    Vector2 operator-=(ScalarType auto scalar)
    {
        *this = *this - CAST(Real, scalar);
        return *this;
    }

    Vector2 operator*=(ScalarType auto scalar)
    {
        *this = *this * CAST(Real, scalar);
        return *this;
    }

    Vector2 operator/=(ScalarType auto scalar)
    {
        *this = *this / CAST(Real, scalar);
        return *this;
    }

    Vector2(glm::vec2 v): X(v.x), Y(v.y) {}

    operator glm::vec2() const
    {
        return { X, Y };
    }
};
}

#if defined(FSN_MATH_USE_GLOBALLY)
using Fussion::Vector2;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Vector2, "Vector2(%.1f, %.1f)", v.X, v.Y)
