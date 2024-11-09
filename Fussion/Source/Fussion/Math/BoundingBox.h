#pragma once
#include <Fussion/Math/Vector3.h>

namespace Fussion {
    struct BoundingBox {
        Vector3 min {}, max {};

        BoundingBox() = default;
        explicit BoundingBox(Vector3 const& center);

        void add_point(Vector3 const& point);
        void add_box(BoundingBox const& box);
        Vector3 center() const;

        BoundingBox translated(Vector3 const& point) const;
        BoundingBox transformed(Mat4 const& matrix) const;

        auto corners() const -> std::array<Vector3, 8>;
    };
}
