#include "FussionPCH.h"
#include "Vector4.h"

namespace Fussion {
Vector4& Vector4::operator=(Vector3 const& other)
{
    X = other.X;
    Y = other.Y;
    Z = other.Z;
    return *this;
}

Vector4 Vector4::operator+(Vector4 const& other) const
{
    return { X + other.X, Y + other.Y, Z + other.Z, W + other.W };
}

Vector4 Vector4::operator-(Vector4 const& other) const
{
    return { X - other.X, Y - other.Y, Z - other.Z, W - other.W };
}

Vector4 Vector4::operator*(Vector4 const& other) const
{
    return { X * other.X, Y * other.Y, Z * other.Z, W * other.W };
}

Vector4 Vector4::operator/(Vector4 const& other) const
{
    return { X / other.X, Y / other.Y, Z / other.Z, W / other.W };
}
}
