#include "Math.h"

namespace Fussion::Math {
    auto FloorLog2(s32 value) -> s32
    {
        return 31 - CountLeadingZeros(CAST(u32, value));
    }

    auto FloorLog2(s64 value) -> s64
    {
        return 61 - CountLeadingZeros(CAST(u64, value));
    }

    auto CountLeadingZeros(u32 value) -> u32
    {
#if _MSC_VER
        return __lzcnt(value);
#else
        return __builtin_clz(value);
#endif
    }

    auto CountLeadingZeros(u64 value) -> u64
    {
#if _MSC_VER
        return __lzcnt64(value);
#else
        return __builtin_clzll(value);
#endif
    }

    auto GetFrustumCornersWorldSpace(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>
    {
        auto inv = glm::inverse(proj * view);
        std::array<Vector4, 8> corners;

        auto i = 0;
        for (auto x = 0; x < 2; x++) {
            for (auto y = 0; y < 2; y++) {
                for (auto z = 0; z < 2; z++) {
                    auto pt = inv * Vector4 { 2 * CAST(f32, x) - 1, 2 * CAST(f32, y) - 1, 2 * CAST(f32, z) - 1, 1.0 };

                    corners[i] = pt / pt.w;

                    i += 1;
                }
            }
        }
        return corners;
    }
}
