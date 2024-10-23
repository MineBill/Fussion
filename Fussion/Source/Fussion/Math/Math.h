#pragma once
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Math/Vector4.h>

#include <complex>

namespace Fussion::Math {

    constexpr f64 PI = 3.14159265358979323846264338327950288;
    constexpr f32 F32_EPSILON = 1.192092896e-07F;
    constexpr f64 F64_EPSILON = 2.2204460492503131e-016;

    template<ScalarType T>
    constexpr auto Min(T first, T second)
    {
        if (first < second)
            return first;
        return second;
    }

    template<ScalarType First, ScalarType Second, ScalarType... T>
    constexpr auto Min(First first, Second second, T... rest) -> First
    {
        auto min_rest = Min(second, rest...);
        if (first < min_rest)
            return first;
        return min_rest;
    }

    template<ScalarType T>
    constexpr auto Max(T first, T second)
    {
        if (first > second)
            return first;
        return second;
    }

    template<ScalarType First, ScalarType Second, ScalarType... T>
    constexpr auto Max(First first, Second second, T... rest) -> First
    {
        auto max_rest = Max(second, rest...);
        if (first > max_rest)
            return first;
        return max_rest;
    }

    template<ScalarType T>
    constexpr auto Clamp(T value, T min, T max)
    {
        if (value < min)
            value = min;
        if (value > max)
            value = max;
        return value;
    }

    template<std::floating_point Real>
    constexpr bool IsZero(Real value)
    {
        if constexpr (std::is_same_v<Real, f32>) {
            return value <= F32_EPSILON;
        } else {
            return value <= F64_EPSILON;
        }
    }

    constexpr auto Abs(ScalarType auto value)
    {
        if (value < 0)
            return -value;
        return value;
    }

    auto Sin(ScalarType auto value) -> decltype(value)
    {
        return std::sin(value);
    }

    auto Cos(ScalarType auto value) -> decltype(value)
    {
        return std::cos(value);
    }

    auto Lerp(ScalarType auto a, ScalarType auto b, ScalarType auto t)
    {
        return a + (b - a) * t;
    }

    template<ScalarType T>
    auto Pow(T value, T power) -> decltype(value)
    {
        return std::pow(value, power);
    }

    constexpr auto Sqrt(ScalarType auto value)
    {
        return std::sqrt(value);
    }

    constexpr auto Floor(ScalarType auto value)
    {
        return CAST(decltype(value), CAST(s32, value));
    }

    constexpr auto FloorSigned(ScalarType auto value)
    {
        if (value >= 0 || CAST(s32, value) == value) {
            return CAST(s32, value);
        }
        return CAST(s32, value) - 1;
    }

    auto FloorLog2(s32 value) -> s32;
    auto FloorLog2(s64 value) -> s64;

    auto CountLeadingZeros(u32 value) -> u32;
    auto CountLeadingZeros(u64 value) -> u64;

    auto GetFrustumCornersWorldSpace(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>;
}
