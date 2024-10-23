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

    Vector2 Vector2::Clamp(Vector2 self, Vector2 const& min, Vector2 const& max)
    {
        self.x = Math::Clamp(self.x, min.x, max.x);
        self.y = Math::Clamp(self.y, min.y, max.y);
        return self;
    }

    Vector2 Vector2::Max(Vector2 const& min, Vector2 const& max)
    {
        Vector2 v;
        v.x = Math::Max(min.x, max.x);
        v.y = Math::Max(min.y, max.y);
        return v;
    }

    Vector2 Vector2::Min(Vector2 const& min, Vector2 const& max)
    {
        Vector2 v;
        v.x = Math::Min(min.x, max.x);
        v.y = Math::Min(min.y, max.y);
        return v;
    }

    Vector2 Vector2::Abs(Vector2 const& vector2)
    {
        Vector2 v;
        v.x = Math::Abs(vector2.x);
        v.y = Math::Abs(vector2.y);
        return v;
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
