#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector2.h"

namespace Fussion {
struct Vector3 final {
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
            Real X, Y, Z;
        };

        Real Raw[3];
    };
#if OS_WINDOWS
#pragma warning(pop)
#endif

    Vector3(): X(0), Y(0), Z(0) {}
    explicit Vector3(ScalarType auto x): X(CAST(Real, x)), Y(0), Z(0) {}
    Vector3(ScalarType auto x, ScalarType auto y): X(CAST(Real, x)), Y(CAST(Real, y)), Z(0) {}
    Vector3(ScalarType auto x, ScalarType auto y, ScalarType auto z): X(CAST(Real, x)), Y(CAST(Real, y)), Z(CAST(Real, z)) {}

    explicit Vector3(Vector2 const& v): X(v.X), Y(v.Y), Z(0) {}
    explicit Vector3(Vector2 const& v, ScalarType auto z): X(v.X), Y(v.Y), Z(z) {}

    operator Vector2() const
    {
        return { X, Y };
    }

    [[nodiscard]] Real LengthSquared() const;

    [[nodiscard]] Real Length() const;

    [[nodiscard]] Vector3 Normalized() const;

    void Normalize();

    Real& operator[](std::size_t i)
    {
        return Raw[i];
    }

    Real operator[](std::size_t i) const
    {
        return Raw[i];
    }

    Vector3 operator+(Vector3 const& other) const;
    Vector3 operator-(Vector3 const& other) const;
    Vector3 operator*(Vector3 const& other) const;
    Vector3 operator/(Vector3 const& other) const;

    Vector3 operator-() const
    {
        return {-X, -Y, -Z};
    }

    Vector3& operator+=(Vector3 const& other);
    Vector3& operator-=(Vector3 const& other);
    Vector3& operator*=(Vector3 const& other);
    Vector3& operator/=(Vector3 const& other);

    Vector3 operator+(ScalarType auto scalar) const
    {
        return { X + CAST(Real, scalar), Y + CAST(Real, scalar), Z + CAST(Real, scalar) };
    }

    Vector3 operator-(ScalarType auto scalar) const
    {
        return { X - CAST(Real, scalar), Y - CAST(Real, scalar), Z - CAST(Real, scalar) };
    }

    Vector3 operator*(ScalarType auto scalar) const
    {
        return { X * CAST(Real, scalar), Y * CAST(Real, scalar), Z * CAST(Real, scalar) };
    }

    Vector3 operator/(ScalarType auto scalar) const
    {
        return { X / CAST(Real, scalar), Y / CAST(Real, scalar), Z / CAST(Real, scalar) };
    }

    Vector3& operator+=(ScalarType auto scalar)
    {
        *this = *this + CAST(Real, scalar);
        return *this;
    }

    Vector3& operator-=(ScalarType auto scalar)
    {
        *this = *this - CAST(Real, scalar);
        return *this;
    }

    Vector3& operator*=(ScalarType auto scalar)
    {
        *this = *this * CAST(Real, scalar);
        return *this;
    }

    Vector3& operator/=(ScalarType auto scalar)
    {
        *this = *this / CAST(Real, scalar);
        return *this;
    }

    Vector3(glm::vec3 v): X(v.x), Y(v.y), Z(v.z) {}

    operator glm::vec3() const
    {
        return { X, Y, Z };
    }

    static const Vector3 One;
    static const Vector3 Up;
    static const Vector3 Down;
    static const Vector3 Left;
    static const Vector3 Right;
    static const Vector3 Forward;
};

bool operator==(Vector3 const& lhs, Vector3 const& rhs);
bool operator!=(Vector3 const& lhs, Vector3 const& rhs);
}

#if defined(FSN_MATH_USE_GLOBALLY)
using Fussion::Vector3;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Vector3, "Vector3({:.2f}, {:.2f}, {:.2f})", v.X, v.Y, v.Z)
