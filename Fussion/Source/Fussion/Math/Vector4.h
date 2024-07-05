#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector3.h"

namespace Fussion {
struct Vector4 final {
#if USE_VECTOR_F64
    using Real = f64;
#else
    using Real = f32;
#endif

    union {
        struct {
            Real X, Y, Z, W;
        };

        Real Raw[4];
    };

    Vector4(): X(0), Y(0), Z(0), W(0) {}
    explicit Vector4(ScalarType auto x): X(x), Y(0), Z(0), W(0) {}
    Vector4(ScalarType auto x, ScalarType auto y): X(x), Y(y), Z(0), W(0) {}
    Vector4(ScalarType auto x, ScalarType auto y, ScalarType auto z): X(x), Y(y), Z(z), W(0) {}
    Vector4(ScalarType auto x, ScalarType auto y, ScalarType auto z, ScalarType auto w): X(x), Y(y), Z(z), W(w) {}

    explicit Vector4(Vector2 v): X(v.X), Y(v.Y), Z(0), W(0) {}
    Vector4(Vector2 v, ScalarType auto z, ScalarType auto w): X(v.X), Y(v.Y), Z(z), W(w) {}

    explicit Vector4(Vector3 const& v): X(v.X), Y(v.Y), Z(v.Z), W(0) {}
    Vector4(Vector3 const& v, ScalarType auto w): X(v.X), Y(v.Y), Z(v.Z), W(w) {}

    // Vector4(Vector4 const& v): X(v.X), Y(v.Y), Z(v.Z), W(v.W) {}

    operator Vector3() const
    {
        return { X, Y, Z };
    }

    operator Vector2() const
    {
        return { X, Y };
    }

    Real& operator[](std::size_t i)
    {
        return Raw[i];
    }

    Real operator[](std::size_t i) const
    {
        return Raw[i];
    }

    Vector4 operator+(Vector4 const& other) const;
    Vector4 operator-(Vector4 const& other) const;
    Vector4 operator*(Vector4 const& other) const;
    Vector4 operator/(Vector4 const& other) const;

    Vector4 operator+(ScalarType auto scalar) const
    {
        return { X + CAST(Real, scalar), Y + CAST(Real, scalar), Z + CAST(Real, scalar), W + CAST(Real, scalar) };
    }

    Vector4 operator-(ScalarType auto scalar) const
    {
        return { X - CAST(Real, scalar), Y - CAST(Real, scalar), Z - CAST(Real, scalar), W - CAST(Real, scalar) };
    }

    Vector4 operator*(ScalarType auto scalar) const
    {
        return { X * CAST(Real, scalar), Y * CAST(Real, scalar), Z * CAST(Real, scalar), W * CAST(Real, scalar) };
    }

    Vector4 operator/(ScalarType auto scalar) const
    {
        return { X / CAST(Real, scalar), Y / CAST(Real, scalar), Z / CAST(Real, scalar), W / CAST(Real, scalar) };
    }

    Vector4 operator+=(ScalarType auto scalar)
    {
        *this = *this + CAST(Real, scalar);
        return *this;
    }

    Vector4 operator-=(ScalarType auto scalar)
    {
        *this = *this - CAST(Real, scalar);
        return *this;
    }

    Vector4 operator*=(ScalarType auto scalar)
    {
        *this = *this * CAST(Real, scalar);
        return *this;
    }

    Vector4 operator/=(ScalarType auto scalar)
    {
        *this = *this / CAST(Real, scalar);
        return *this;
    }

    Vector4(glm::vec4 v): X(v.x), Y(v.y), Z(v.z), W(v.w) {}

    operator glm::vec4() const
    {
        return { X, Y, Z, W };
    }
};
}

#if defined(FSN_MATH_USE_GLOBALLY)
using Fussion::Vector4;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Vector4, "Vector4(%.1f, %.1f, %.1f, %.1f)", v.X, v.Y, v.Z, v.W)