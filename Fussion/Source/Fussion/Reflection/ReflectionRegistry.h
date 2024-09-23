#pragma once

namespace Fussion {
    class ReflectionRegistry {
    public:
        ReflectionRegistry();
        static void Register();
    };
}

#define REGISTER_ENUM(EnumName)                                                 \
{                                                                               \
    auto ee = meta::enum_<EnumName>(meta::metadata_()("Name"s, #EnumName##s));  \
    for (auto const& [value, name] : magic_enum::enum_entries<EnumName>()) {    \
        ee.evalue_(std::string{ name }, value);                                 \
    }                                                                           \
}