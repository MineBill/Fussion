#include "FussionPCH.h"
#include "Vector2.h"

#include "Math.h"

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

    Vector2 Vector2::clamp(Vector2 self, Vector2 const& min, Vector2 const& max)
    {
        self.x = Math::clamp(self.x, min.x, max.x);
        self.y = Math::clamp(self.y, min.y, max.y);
        return self;
    }

    Vector2 Vector2::max(Vector2 const& min, Vector2 const& max)
    {
        Vector2 v;
        v.x = Math::max(min.x, max.x);
        v.y = Math::max(min.y, max.y);
        return v;
    }

    Vector2 Vector2::min(Vector2 const& min, Vector2 const& max)
    {
        Vector2 v;
        v.x = Math::min(min.x, max.x);
        v.y = Math::min(min.y, max.y);
        return v;
    }

    Vector2 Vector2::abs(Vector2 const& vector2)
    {
        Vector2 v;
        v.x = Math::abs(vector2.x);
        v.y = Math::abs(vector2.y);
        return v;
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
