#include "BoundingBox.h"

namespace Fussion {
    BoundingBox::BoundingBox(Vector3 const& center)
        : Min(center)
        , Max(center)
    {
    }

    void BoundingBox::IncludePoint(Vector3 const& point)
    {
        Max.x = Math::Max(Max.x, point.x);
        Max.y = Math::Max(Max.y, point.y);
        Max.z = Math::Max(Max.z, point.z);

        Min.x = Math::Min(Min.x, point.x);
        Min.y = Math::Min(Min.y, point.y);
        Min.z = Math::Min(Min.z, point.z);
    }

    void BoundingBox::Include(BoundingBox const& box)
    {
        IncludePoint(box.Min);
        IncludePoint(box.Max);
    }

    Vector3 BoundingBox::Center() const
    {
        return (Min + Max) / 2.0f;
    }

    BoundingBox BoundingBox::Translated(Vector3 const& point) const
    {
        BoundingBox box = *this;
        box.Min += point;
        box.Max += point;
        return box;
    }

    BoundingBox BoundingBox::Transformed(Mat4 const& matrix) const
    {
        BoundingBox box = *this;
        box.Min = Vector3(matrix * Vector4(box.Min, 1.0f));
        box.Max = Vector3(matrix * Vector4(box.Max, 1.0f));
        return box;
    }

    auto BoundingBox::GetCorners() const -> std::array<Vector3, 8>
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
