#include "e5pch.h"
#include "Vector3.h"

namespace Fussion {
Vector3::Real Vector3::LengthSquared() const {
    return X * X + Y * Y + Z * Z;
}

Vector3::Real Vector3::Length() const {
    return std::sqrt(LengthSquared());
}

Vector3 Vector3::Normalized() const {
    return *this / Length();
}

void Vector3::Normalize() {
    *this = this->Normalized();
}

Vector3 Vector3::operator+(Vector3 const& other) const
{
    return { X + other.X, Y + other.Y, Z + other.Z };
}

Vector3 Vector3::operator-(Vector3 const& other) const
{
    return { X - other.X, Y - other.Y, Z - other.Z };
}

Vector3 Vector3::operator*(Vector3 const& other) const
{
    return { X * other.X, Y * other.Y, Z * other.Z };
}

Vector3 Vector3::operator/(Vector3 const& other) const
{
    return { X / other.X, Y / other.Y, Z / other.Z };
}

Vector3 Vector3::operator+=(Vector3 const& other)
{
    *this = *this + other;
    return *this;
}

Vector3 Vector3::operator-=(Vector3 const& other)
{
    *this = *this - other;
    return *this;
}

Vector3 Vector3::operator*=(Vector3 const& other)
{
    *this = *this * other;
    return *this;
}

Vector3 Vector3::operator/=(Vector3 const& other)
{
    *this = *this / other;
    return *this;
}
}
