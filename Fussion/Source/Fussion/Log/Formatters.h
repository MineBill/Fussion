#pragma once
#include "Fussion/Core/Types.h"

#define FSN_MAKE_FORMATTABLE(Type, Format, ...)                         \
    template<>                                                          \
    struct std::formatter<Type>                                         \
    {                                                                   \
        constexpr auto parse(auto& context) { return context.begin(); } \
                                                                        \
        template<typename FormatContext>                                \
        auto format(const Type& v, FormatContext& context) const        \
        {                                                               \
            return format_to(context.out(), Format, __VA_ARGS__);       \
        }                                                               \
    };
