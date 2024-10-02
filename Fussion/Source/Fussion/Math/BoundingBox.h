#pragma once
#include <Fussion/Math/Vector3.h>

namespace Fussion {
    struct BoundingBox {
        Vector3 Min {}, Max {};

        BoundingBox() = default;
        explicit BoundingBox(Vector3 const& center);

        void include_point(Vector3 const& point);
        void include(BoundingBox const& box);
        Vector3 center() const;

        BoundingBox translated(Vector3 const& point) const;
        BoundingBox transformed(Mat4 const& matrix) const;

        auto get_corners() const -> std::array<Vector3, 8>;
    };
}
