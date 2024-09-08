#include "FussionPCH.h"
#include "Vector2.h"
#include "Math/Math.h"

namespace Fussion {
    Vector2::Real Vector2::length() const
    {
        return Math::abs(Math::sqrt(x * x + y * y));
    }

    Vector2::Real Vector2::length_squared() const
    {
        return x * x + y * y;
    }

    Vector2::Real Vector2::distance_to(Vector2 const& other) const
    {
        return (*this - other).length();
    }

    Vector2::Real Vector2::distance_to_squared(Vector2 const& other) const
    {
        return (*this - other).length_squared();
    }

    Vector2 Vector2::operator+(Vector2 const& right) const
    {
        return { x + right.x, y + right.y };
    }

    Vector2 Vector2::operator-(Vector2 const& right) const
    {
        return { x - right.x, y - right.y };
    }

    Vector2 Vector2::operator*(Vector2 const& right) const
    {
        return { x * right.x, y * right.y };
    }

    Vector2 Vector2::operator/(Vector2 const& right) const
    {
        return { x / right.x, y / right.y };
    }

    bool operator==(Vector2 const& lhs, Vector2 const& rhs)
    {
        return Math::is_zero(Math::abs(lhs.x - rhs.x)) && Math::is_zero(Math::abs(lhs.y - rhs.y));
    }

    bool operator!=(Vector2 const& lhs, Vector2 const& rhs)
    {
        return !(lhs == rhs);
    }

    bool Vector2::is_zero() const
    {
        return Math::is_zero(x) && Math::is_zero(y);
    }

    f32 Vector2::aspect() const
    {
        return x / y;
    }

}
