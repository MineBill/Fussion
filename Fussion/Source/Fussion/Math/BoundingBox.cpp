#include "BoundingBox.h"

namespace Fussion {
    BoundingBox::BoundingBox(Vector3 const& center)
        : Min(center)
        , Max(center)
    {
    }

    void BoundingBox::include_point(Vector3 const& point)
    {
        Max.x = Math::max(Max.x, point.x);
        Max.y = Math::max(Max.y, point.y);
        Max.z = Math::max(Max.z, point.z);

        Min.x = Math::min(Min.x, point.x);
        Min.y = Math::min(Min.y, point.y);
        Min.z = Math::min(Min.z, point.z);
    }

    void BoundingBox::include(BoundingBox const& box)
    {
        include_point(box.Min);
        include_point(box.Max);
    }

    Vector3 BoundingBox::center() const
    {
        return (Min + Max) / 2.0f;
    }

    BoundingBox BoundingBox::translated(Vector3 const& point) const
    {
        BoundingBox box = *this;
        box.Min += point;
        box.Max += point;
        return box;
    }

    BoundingBox BoundingBox::transformed(Mat4 const& matrix) const
    {
        BoundingBox box = *this;
        box.Min = Vector3(matrix * Vector4(box.Min, 1.0f));
        box.Max = Vector3(matrix * Vector4(box.Max, 1.0f));
        return box;
    }

    auto BoundingBox::get_corners() const -> std::array<Vector3, 8>
    {
        return {
            Min,
            Min + Vector3(1, 0, 0),
            Min + Vector3(0, 1, 0),
            Min + Vector3(0, 0, 1),
            Max,
            Max - Vector3(1, 0, 0),
            Max - Vector3(0, 1, 0),
            Max - Vector3(0, 0, 1),
        };
    }
}
