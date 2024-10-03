#include "FussionPCH.h"
#include "Vector3.h"

#include "Math.h"

namespace Fussion {
    Vector3::Real Vector3::LengthSquared() const
    {
        return x * x + y * y + z * z;
    }

    Vector3::Real Vector3::Length() const
    {
        return std::sqrt(LengthSquared());
    }

    Vector3 Vector3::Normalized() const
    {
        return *this / Length();
    }

    void Vector3::Normalize()
    {
        *this = this->Normalized();
    }

    Vector3 Vector3::operator+(Vector3 const& other) const
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vector3 Vector3::operator-(Vector3 const& other) const
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    Vector3 Vector3::operator*(Vector3 const& other) const
    {
        return { x * other.x, y * other.y, z * other.z };
    }

    Vector3 Vector3::operator/(Vector3 const& other) const
    {
        return { x / other.x, y / other.y, z / other.z };
    }

    Vector3& Vector3::operator+=(Vector3 const& other)
    {
        *this = *this + other;
        return *this;
    }

    Vector3& Vector3::operator-=(Vector3 const& other)
    {
        *this = *this - other;
        return *this;
    }

    Vector3& Vector3::operator*=(Vector3 const& other)
    {
        *this = *this * other;
        return *this;
    }

    Vector3& Vector3::operator/=(Vector3 const& other)
    {
        *this = *this / other;
        return *this;
    }

    bool operator==(Vector3 const& lhs, Vector3 const& rhs)
    {
        return Math::IsZero(Math::Abs(lhs.x - rhs.x)) && Math::IsZero(Math::Abs(lhs.y - rhs.y)) && Math::IsZero(Math::Abs(lhs.z - rhs.z));
    }

    bool operator!=(Vector3 const& lhs, Vector3 const& rhs)
    {
        return !(lhs == rhs);
    }

    Vector3 const Vector3::One { 1, 1, 1 };
    Vector3 const Vector3::Up { 0, 1, 0 };
    Vector3 const Vector3::Down { 0, -1, 0 };
    Vector3 const Vector3::Left { -1, 0, 0 };
    Vector3 const Vector3::Right { 1, 0, 0 };
    Vector3 const Vector3::Forward { 0, 0, 1 };

}
