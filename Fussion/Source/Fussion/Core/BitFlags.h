#pragma once

#define DECLARE_FLAGS(Enum, Flags)                                          \
    struct Flags {                                                          \
        using EnumType = Enum;                                              \
        inline constexpr Flags() = default;                                 \
        inline constexpr Flags(Enum v) : value(int(v)) {}                   \
        inline constexpr bool Test(Enum e) const { return value & int(e); } \
        inline constexpr operator int() const { return value; }             \
        int value{};                                                        \
    };

#define DECLARE_OPERATORS_FOR_FLAGS(Flags)                                                                           \
    inline Flags operator|(Flags::EnumType a, Flags::EnumType b) { return Flags(Flags::EnumType(int(a) | int(b))); } \
    inline Flags operator|(Flags a, Flags::EnumType b) { return Flags(Flags::EnumType(a.value | int(b))); }          \
    inline Flags operator|(Flags::EnumType a, Flags b) { return Flags(Flags::EnumType(int(a) | b.value)); }          \
    inline Flags operator|(Flags a, Flags b) { return Flags(Flags::EnumType(a.value | b.value)); }                   \
    inline Flags operator|=(Flags a, Flags::EnumType b) { return a = Flags(Flags::EnumType(a.value | int(b))); }
