#pragma once
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>

namespace Fussion {
    struct Vector4 final {
#if USE_VECTOR_F64
        using Real = f64;
#else
        using Real = f32;
#endif

#if OS_WINDOWS
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
        union {
            struct {
                Real x, y, z, w;
            };

            Real raw[4];
        };
#if OS_WINDOWS
#    pragma warning(pop)
#endif

        Vector4()
            : x(0)
            , y(0)
            , z(0)
            , w(0)
        { }
        explicit Vector4(ScalarType auto x)
            : x(CAST(Real, x))
            , y(0)
            , z(0)
            , w(0)
        { }
        Vector4(ScalarType auto x, ScalarType auto y)
            : x(CAST(Real, x))
            , y(CAST(Real, y))
            , z(0)
            , w(0)
        { }
        Vector4(ScalarType auto x, ScalarType auto y, ScalarType auto z)
            : x(CAST(Real, x))
            , y(CAST(Real, y))
            , z(CAST(Real, z))
            , w(0)
        { }
        Vector4(ScalarType auto x, ScalarType auto y, ScalarType auto z, ScalarType auto w)
            : x(CAST(Real, x))
            , y(CAST(Real, y))
            , z(CAST(Real, z))
            , w(CAST(Real, w))
        { }

        explicit Vector4(Vector2 const& v)
            : x(v.x)
            , y(v.y)
            , z(0)
            , w(0)
        { }
        Vector4(Vector2 v, ScalarType auto z, ScalarType auto w)
            : x(v.x)
            , y(v.y)
            , z(z)
            , w(w)
        { }

        explicit Vector4(Vector3 const& v)
            : x(v.x)
            , y(v.y)
            , z(v.z)
            , w(0)
        { }
        Vector4(Vector3 const& v, ScalarType auto w)
            : x(v.x)
            , y(v.y)
            , z(v.z)
            , w(w)
        { }

        // Vector4(Vector4 const& v): X(v.X), Y(v.Y), Z(v.Z), W(v.W) {}

        operator Vector3() const
        {
            return { x, y, z };
        }

        operator Vector2() const
        {
            return { x, y };
        }

        Real& operator[](std::size_t i)
        {
            return raw[i];
        }

        Real operator[](std::size_t i) const
        {
            return raw[i];
        }

        Vector4& operator=(Vector3 const& other);

        Vector4 operator+(Vector4 const& other) const;
        Vector4 operator-(Vector4 const& other) const;
        Vector4 operator*(Vector4 const& other) const;
        Vector4 operator/(Vector4 const& other) const;

        Vector4 operator+(ScalarType auto scalar) const
        {
            return { x + CAST(Real, scalar), y + CAST(Real, scalar), z + CAST(Real, scalar), w + CAST(Real, scalar) };
        }

        Vector4 operator-(ScalarType auto scalar) const
        {
            return { x - CAST(Real, scalar), y - CAST(Real, scalar), z - CAST(Real, scalar), w - CAST(Real, scalar) };
        }

        Vector4 operator*(ScalarType auto scalar) const
        {
            return { x * CAST(Real, scalar), y * CAST(Real, scalar), z * CAST(Real, scalar), w * CAST(Real, scalar) };
        }

        Vector4 operator/(ScalarType auto scalar) const
        {
            return { x / CAST(Real, scalar), y / CAST(Real, scalar), z / CAST(Real, scalar), w / CAST(Real, scalar) };
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

        Vector4(glm::vec4 v)
            : x(v.x)
            , y(v.y)
            , z(v.z)
            , w(v.w)
        { }

        operator glm::vec4() const
        {
            return { x, y, z, w };
        }
    };
}

#if defined(FSN_MATH_USE_GLOBALLY)
using Fussion::Vector4;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Vector4, "Vector4({:.2f}, {:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z, v.w)
