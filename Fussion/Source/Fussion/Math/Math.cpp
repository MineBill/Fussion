#include "Math.h"

namespace Fussion::Math {
    auto GetFrustumCornersWorldSpace(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>
    {
        auto inv = glm::inverse(proj * view);
        std::array<Vector4, 8> corners;

        auto i = 0;
        for (auto x = 0; x < 2; x++) {
            for (auto y = 0; y < 2; y++) {
                for (auto z = 0; z < 2; z++) {
                    auto pt = inv * Vector4{
                        2 * CAST(f32, x) - 1,
                        2 * CAST(f32, y) - 1,
                        2 * CAST(f32, z) - 1,
                        1.0 };

                    corners[i] = pt / pt.w;

                    i += 1;
                }
            }
        }
        return corners;
    }
}
