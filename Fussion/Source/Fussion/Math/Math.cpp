#include "Math.h"

namespace Fussion::Math {
    auto floor_log2(s32 value) -> s32
    {
        return 31 - count_leading_zeros(CAST(u32, value));
    }

    auto floor_log2(s64 value) -> s64
    {
        return 61 - count_leading_zeros(CAST(u64, value));
    }

    auto count_leading_zeros(u32 value) -> u32
    {
#if _MSC_VER
        return __lzcnt(value);
#else
        return __builtin_clz(value);
#endif
    }

    auto count_leading_zeros(u64 value) -> u64
    {
#if _MSC_VER
        return __lzcnt64(value);
#else
        return __builtin_clzll(value);
#endif
    }

    auto get_frustum_corners_world_space(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>
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
