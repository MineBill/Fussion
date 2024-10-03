#pragma once
#include <Fussion/Math/Vector3.h>

namespace Fussion {
    struct BoundingBox {
        Vector3 Min {}, Max {};

        BoundingBox() = default;
        explicit BoundingBox(Vector3 const& center);

        void IncludePoint(Vector3 const& point);
        void Include(BoundingBox const& box);
        Vector3 Center() const;

        BoundingBox Translated(Vector3 const& point) const;
        BoundingBox Transformed(Mat4 const& matrix) const;

        auto GetCorners() const -> std::array<Vector3, 8>;
    };
}
