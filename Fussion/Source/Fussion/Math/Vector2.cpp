﻿#include "W:\source\projects\Fussion\build\.gens\Fussion\windows\x64\debug\Fussion\Source\e5pch.h"
#include "Vector2.h"

namespace Fussion {
Vector2::Real Vector2::Length() const {
    return std::abs(std::sqrt(X * X + Y * Y));
}

Vector2::Real Vector2::LengthSquared() const {
    return X * X + Y * Y;
}

Vector2::Real Vector2::DistanceTo(Vector2 const& other) const {
    return (*this - other).Length();
}

Vector2::Real Vector2::DistanceToSquared(Vector2 const& other) const {
    return (*this - other).LengthSquared();
}

Vector2 Vector2::operator+(Vector2 const& right) const
{
    return { X + right.X, Y + right.Y };
}

Vector2 Vector2::operator-(Vector2 const& right) const
{
    return { X - right.X, Y - right.Y };
}

Vector2 Vector2::operator*(Vector2 const& right) const
{
    return { X * right.X, Y * right.Y };
}

Vector2 Vector2::operator/(Vector2 const& right) const
{
    return { X / right.X, Y / right.Y };
}
}