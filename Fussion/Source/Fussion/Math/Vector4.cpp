﻿#include "W:\source\projects\Fussion\build\.gens\Fussion\windows\x64\debug\Fussion\Source\e5pch.h"
#include "Vector4.h"

namespace Fussion {
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