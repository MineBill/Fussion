#pragma once
#include <Fussion/Math/Vector4.h>
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Types.h>

#include <complex>

namespace Fussion::Math {

    constexpr f64 PI = 3.14159265358979323846264338327950288;
    constexpr f32 F32_EPSILON = 1.192092896e-07F;
    constexpr f64 F64_EPSILON = 2.2204460492503131e-016;

    template<ScalarType T>
    constexpr auto min(T first, T second)
    {
        if (first < second)
            return first;
        return second;
    }

    template<ScalarType First, ScalarType Second, ScalarType... T>
    constexpr auto min(First first, Second second, T... rest) -> First
    {
        auto min_rest = min(second, rest...);
        if (first < min_rest)
            return first;
        return min_rest;
    }

    template<ScalarType T>
    constexpr auto max(T first, T second)
    {
        if (first > second)
            return first;
        return second;
    }

    template<ScalarType First, ScalarType Second, ScalarType... T>
    constexpr auto max(First first, Second second, T... rest) -> First
    {
        auto max_rest = max(second, rest...);
        if (first > max_rest)
            return first;
        return max_rest;
    }

    template<ScalarType T>
    constexpr auto clamp(T value, T min, T max)
    {
        if (value < min)
            value = min;
        if (value > max)
            value = max;
        return value;
    }

    template<std::floating_point Real>
    constexpr bool is_zero(Real value)
    {
        if constexpr (std::is_same_v<Real, f32>) {
            return value <= F32_EPSILON;
        } else {
            return value <= F64_EPSILON;
        }
    }

    constexpr auto abs(ScalarType auto value)
    {
        if (value < 0)
            return -value;
        return value;
    }

    auto sin(ScalarType auto value) -> decltype(value)
    {
        return std::sin(value);
    }

    auto cos(ScalarType auto value) -> decltype(value)
    {
        return std::cos(value);
    }

    auto lerp(ScalarType auto a, ScalarType auto b, ScalarType auto t)
    {
        return a + (b - a) * t;
    }

    template<ScalarType T>
    auto pow(T value, T power) -> decltype(value)
    {
        return std::pow(value, power);
    }

    constexpr auto sqrt(ScalarType auto value)
    {
        return std::sqrt(value);
    }

    constexpr auto floor(ScalarType auto value)
    {
        return CAST(decltype(value), CAST(s32, value));
    }

    auto floor_log2(s32 value) -> s32;
    auto floor_log2(s64 value) -> s64;

    auto count_leading_zeros(u32 value) -> u32;
    auto count_leading_zeros(u64 value) -> u64;

    auto get_frustum_corners_world_space(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>;
}
