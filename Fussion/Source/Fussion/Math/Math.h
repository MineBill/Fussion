#pragma once
#include "Fussion/Core/Types.h"

#include <complex>

namespace Fussion::Math {

constexpr f64 Pi = 3.14159265358979323846264338327950288;

auto Min(ScalarType auto first, ScalarType auto second) -> decltype(first)
{
    if (first < second)
        return first;
    return second;
}

template<ScalarType First, ScalarType Second, ScalarType... T>
auto Min(First first, Second second, T... rest) -> First
{
    auto min_rest = Min(second, rest...);
    if (first < min_rest)
        return first;
    return min_rest;
}

auto Max(ScalarType auto first, ScalarType auto second) -> decltype(first)
{
    if (first > second)
        return first;
    return second;
}

template<ScalarType First, ScalarType Second, ScalarType... T>
auto Max(First first, Second second, T... rest) -> First
{
    auto max_rest = Max(second, rest...);
    if (first > max_rest)
        return first;
    return max_rest;
}

auto Clamp(ScalarType auto value, ScalarType auto min, ScalarType auto max) -> decltype(value)
{
    if (value < min)
        value = min;
    if (value > max)
        value = max;
    return value;
}

template<std::floating_point Real>
bool IsZero(Real value)
{
    if constexpr (std::is_same_v<Real, f32>) {
        constexpr f32 epsilon = FLT_EPSILON;
        return value <= epsilon;
    } else {
        constexpr f64 epsilon = DBL_EPSILON;
        return value <= epsilon;
    }
}

auto Sin(ScalarType auto value) -> decltype(value)
{
    return std::sin(value);
}

auto Cos(ScalarType auto value) -> decltype(value)
{
    return std::cos(value);
}

auto Pow(ScalarType auto value, ScalarType auto power) -> decltype(value)
{
    return std::pow(value, power);
}

}
