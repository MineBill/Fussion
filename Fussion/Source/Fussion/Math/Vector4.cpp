#include "FussionPCH.h"
#include "Vector4.h"

namespace Fussion {
    Vector4& Vector4::operator=(Vector3 const& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    Vector4 Vector4::operator+(Vector4 const& other) const
    {
        return { x + other.x, y + other.y, z + other.z, w + other.w };
    }

    Vector4 Vector4::operator-(Vector4 const& other) const
    {
        return { x - other.x, y - other.y, z - other.z, w - other.w };
    }

    Vector4 Vector4::operator*(Vector4 const& other) const
    {
        return { x * other.x, y * other.y, z * other.z, w * other.w };
    }

    Vector4 Vector4::operator/(Vector4 const& other) const
    {
        return { x / other.x, y / other.y, z / other.z, w / other.w };
    }
}
