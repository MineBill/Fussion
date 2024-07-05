#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion::Math {
    auto Min(ScalarType auto first, ScalarType auto second) -> decltype(first)
    {
        if (first < second)
            return first;
        return second;
    }

    auto Max(ScalarType auto first, ScalarType auto second) -> decltype(first)
    {
        if (first > second)
            return first;
        return second;
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
