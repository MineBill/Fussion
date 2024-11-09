#include "BoundingBox.h"

namespace Fussion {
    BoundingBox::BoundingBox(Vector3 const& center)
        : min(center)
        , max(center)
    {
    }

    void BoundingBox::add_point(Vector3 const& point)
    {
        max.x = Math::max(max.x, point.x);
        max.y = Math::max(max.y, point.y);
        max.z = Math::max(max.z, point.z);

        min.x = Math::min(min.x, point.x);
        min.y = Math::min(min.y, point.y);
        min.z = Math::min(min.z, point.z);
    }

    void BoundingBox::add_box(BoundingBox const& box)
    {
        add_point(box.min);
        add_point(box.max);
    }

    Vector3 BoundingBox::center() const
    {
        return (min + max) / 2.0f;
    }

    BoundingBox BoundingBox::translated(Vector3 const& point) const
    {
        BoundingBox box = *this;
        box.min += point;
        box.max += point;
        return box;
    }

    BoundingBox BoundingBox::transformed(Mat4 const& matrix) const
    {
        BoundingBox box = *this;
        box.min = Vector3(matrix * Vector4(box.min, 1.0f));
        box.max = Vector3(matrix * Vector4(box.max, 1.0f));
        return box;
    }

    auto BoundingBox::corners() const -> std::array<Vector3, 8>
    {
        return {
            min,
            min + Vector3(1, 0, 0),
            min + Vector3(0, 1, 0),
            min + Vector3(0, 0, 1),
            max,
            max - Vector3(1, 0, 0),
            max - Vector3(0, 1, 0),
            max - Vector3(0, 0, 1),
        };
    }
}
