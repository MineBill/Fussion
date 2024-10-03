#pragma once
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Math/Vector2.h>

namespace Fussion {
    struct Vector3 final {
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
                Real x, y, z;
            };

            Real raw[3];
        };
#if OS_WINDOWS
#    pragma warning(pop)
#endif

        Vector3()
            : x(0)
            , y(0)
            , z(0)
        {
        }
        explicit Vector3(ScalarType auto x)
            : x(CAST(Real, x))
            , y(0)
            , z(0)
        {
        }
        Vector3(ScalarType auto x, ScalarType auto y)
            : x(CAST(Real, x))
            , y(CAST(Real, y))
            , z(0)
        {
        }
        Vector3(ScalarType auto x, ScalarType auto y, ScalarType auto z)
            : x(CAST(Real, x))
            , y(CAST(Real, y))
            , z(CAST(Real, z))
        {
        }

        explicit Vector3(Vector2 const& v)
            : x(v.x)
            , y(v.y)
            , z(0)
        {
        }
        explicit Vector3(Vector2 const& v, ScalarType auto z)
            : x(v.x)
            , y(v.y)
            , z(CAST(Real, z))
        {
        }

        operator Vector2() const
        {
            return { x, y };
        }

        [[nodiscard]]
        Real Length() const;

        [[nodiscard]]
        Real LengthSquared() const;

        [[nodiscard]]
        Vector3 Normalized() const;

        void Normalize();

        Real& operator[](std::size_t i)
        {
            return raw[i];
        }

        Real operator[](std::size_t i) const
        {
            return raw[i];
        }

        Vector3 operator+(Vector3 const& other) const;
        Vector3 operator-(Vector3 const& other) const;
        Vector3 operator*(Vector3 const& other) const;
        Vector3 operator/(Vector3 const& other) const;

        Vector3 operator-() const
        {
            return { -x, -y, -z };
        }

        Vector3& operator+=(Vector3 const& other);
        Vector3& operator-=(Vector3 const& other);
        Vector3& operator*=(Vector3 const& other);
        Vector3& operator/=(Vector3 const& other);

        Vector3 operator+(ScalarType auto scalar) const
        {
            return { x + CAST(Real, scalar), y + CAST(Real, scalar), z + CAST(Real, scalar) };
        }

        Vector3 operator-(ScalarType auto scalar) const
        {
            return { x - CAST(Real, scalar), y - CAST(Real, scalar), z - CAST(Real, scalar) };
        }

        Vector3 operator*(ScalarType auto scalar) const
        {
            return { x * CAST(Real, scalar), y * CAST(Real, scalar), z * CAST(Real, scalar) };
        }

        Vector3 operator/(ScalarType auto scalar) const
        {
            return { x / CAST(Real, scalar), y / CAST(Real, scalar), z / CAST(Real, scalar) };
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

        Vector3(glm::vec3 v)
            : x(v.x)
            , y(v.y)
            , z(v.z)
        {
        }

        operator glm::vec3() const
        {
            return { x, y, z };
        }

        static Vector3 const One;
        static Vector3 const Up;
        static Vector3 const Down;
        static Vector3 const Left;
        static Vector3 const Right;
        static Vector3 const Forward;
    };

    bool operator==(Vector3 const& lhs, Vector3 const& rhs);
    bool operator!=(Vector3 const& lhs, Vector3 const& rhs);
}

#if defined(FSN_MATH_USE_GLOBALLY)
using Fussion::Vector3;
#endif

FSN_MAKE_FORMATTABLE(Fussion::Vector3, "Vector3({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z)
