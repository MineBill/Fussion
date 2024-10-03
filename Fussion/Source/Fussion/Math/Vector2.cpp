#include "FussionPCH.h"
#include "Vector2.h"

#include "Math.h"

namespace Fussion {
    Vector2::Real Vector2::Length() const
    {
        return Math::Abs(Math::Sqrt(x * x + y * y));
    }

    Vector2::Real Vector2::LengthSquared() const
    {
        return x * x + y * y;
    }

    Vector2::Real Vector2::DistanceTo(Vector2 const& other) const
    {
        return (*this - other).Length();
    }

    Vector2::Real Vector2::DistanceToSquared(Vector2 const& other) const
    {
        return (*this - other).LengthSquared();
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
        return Math::IsZero(Math::Abs(lhs.x - rhs.x)) && Math::IsZero(Math::Abs(lhs.y - rhs.y));
    }

    bool operator!=(Vector2 const& lhs, Vector2 const& rhs)
    {
        return !(lhs == rhs);
    }

    bool Vector2::IsZero() const
    {
        return Math::IsZero(x) && Math::IsZero(y);
    }

    f32 Vector2::Aspect() const
    {
        return x / y;
    }

}
