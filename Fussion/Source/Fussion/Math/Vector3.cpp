#include "FussionPCH.h"
#include "Vector3.h"

#include "Math.h"

namespace Fussion {
Vector3::Real Vector3::length_squared() const
{
    return x * x + y * y + z * z;
}

Vector3::Real Vector3::length() const
{
    return std::sqrt(length_squared());
}

Vector3 Vector3::normalized() const
{
    return *this / length();
}

void Vector3::normalize()
{
    *this = this->normalized();
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
    return Math::is_zero(Math::abs(lhs.x - rhs.x)) && Math::is_zero(Math::abs(lhs.y - rhs.y)) && Math::is_zero(Math::abs(lhs.z - rhs.z));
}

bool operator!=(Vector3 const& lhs, Vector3 const& rhs)
{
    return !(lhs == rhs);
}

Vector3 const Vector3::One{ 1, 1, 1 };
Vector3 const Vector3::Up{ 0, 1, 0 };
Vector3 const Vector3::Down{ 0, -1, 0 };
Vector3 const Vector3::Left{ -1, 0, 0 };
Vector3 const Vector3::Right{ 1, 0, 0 };
Vector3 const Vector3::Forward{ 0, 0, 1 };

}
