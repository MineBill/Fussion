#include "FussionPCH.h"
#include "Vector2.h"
#include "Math/Math.h"

namespace Fussion {
    Vector2::Real Vector2::Length() const
    {
        return Math::Abs(Math::Sqrt(X * X + Y * Y));
    }

    Vector2::Real Vector2::LengthSquared() const
    {
        return X * X + Y * Y;
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

    bool operator==(Vector2 const& lhs, Vector2 const& rhs)
    {
        return Math::IsZero(Math::Abs(lhs.X - rhs.X)) && Math::IsZero(Math::Abs(lhs.Y - rhs.Y));
    }

    bool operator!=(Vector2 const& lhs, Vector2 const& rhs)
    {
        return !(lhs == rhs);
    }

    bool Vector2::IsZero() const
    {
        return Math::IsZero(X) && Math::IsZero(Y);
    }

    f32 Vector2::Aspect() const
    {
        return X / Y;
    }

}
