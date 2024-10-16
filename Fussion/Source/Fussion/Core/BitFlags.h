#pragma once

#define DECLARE_FLAGS(Enum, Flags)                                                       \
    struct Flags {                                                                       \
        using EnumType = Enum;                                                           \
        inline constexpr Flags() = default;                                              \
        inline constexpr Flags(Enum v)                                                   \
            : value(static_cast<int>(v))                                                 \
        { }                                                                              \
        inline constexpr bool test(Enum e) const { return value & static_cast<int>(e); } \
        inline constexpr operator int() const { return value; }                          \
        inline constexpr void set(Enum e) { value |= static_cast<int>(e); }              \
        inline constexpr void unset(Enum e) { value &= ~static_cast<int>(e); }           \
        inline constexpr void toggle(Enum e)                                             \
        {                                                                                \
            if (test(e)) {                                                               \
                unset(e);                                                                \
            } else {                                                                     \
                set(e);                                                                  \
            }                                                                            \
        }                                                                                \
        int value {};                                                                    \
    };

#define DECLARE_OPERATORS_FOR_FLAGS(Flags)                                                                           \
    inline Flags operator|(Flags::EnumType a, Flags::EnumType b) { return Flags(Flags::EnumType(int(a) | int(b))); } \
    inline Flags operator|(Flags a, Flags::EnumType b) { return Flags(Flags::EnumType(a.value | int(b))); }          \
    inline Flags operator|(Flags::EnumType a, Flags b) { return Flags(Flags::EnumType(int(a) | b.value)); }          \
    inline Flags operator|(Flags a, Flags b) { return Flags(Flags::EnumType(a.value | b.value)); }                   \
    inline Flags operator|=(Flags& a, Flags::EnumType b) { return a = Flags(Flags::EnumType(a.value | int(b))); }

#define BITFLAGS(Enum)               \
    DECLARE_FLAGS(Enum, Enum##Flags) \
    DECLARE_OPERATORS_FOR_FLAGS(Enum##Flags)
