#pragma once
#include "Fussion/Core/Types.h"

template<>
struct std::formatter<Vector2>
{
    constexpr auto parse(auto& context) { return context.begin(); }

    template<typename FormatContext>
    auto format(const Vector2& vec, FormatContext& context) const
    {
        return format_to(context.out(), "({}, {})", vec.x, vec.y);
    }
};

template<>
struct std::formatter<Vector3>
{
    constexpr auto parse(auto& context) { return context.begin(); }

    template<typename FormatContext>
    auto format(const Vector3& vec, FormatContext& context) const
    {
        return format_to(context.out(), "({}, {}, {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct std::formatter<Vector4>
{
    constexpr auto parse(auto& context) { return context.begin(); }

    template<typename FormatContext>
    auto format(const Vector4& vec, FormatContext& context) const
    {
        return format_to(context.out(), "({}, {}, {}, {})", vec.x, vec.y, vec.z, vec.w);
    }
};
