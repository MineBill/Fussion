#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion::Math {

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
}
