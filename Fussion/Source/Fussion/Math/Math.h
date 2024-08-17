#pragma once
#include <Fussion/Math/Vector4.h>
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Types.h>

#include <complex>

namespace Fussion::Math {

    constexpr f64 Pi = 3.14159265358979323846264338327950288;
    constexpr f32 F32Epsilon = 1.192092896e-07F;
    constexpr f64 F64Epsilon = 2.2204460492503131e-016;

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
            return value <= F32Epsilon;
        } else {
            return value <= F64Epsilon;
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

    template<ScalarType T>
    auto Pow(T value, T power) -> decltype(value)
    {
        return std::pow(value, power);
    }

    constexpr auto Sqrt(ScalarType auto value)
    {
        return std::sqrt(value);
    }

    auto GetFrustumCornersWorldSpace(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>;
}
