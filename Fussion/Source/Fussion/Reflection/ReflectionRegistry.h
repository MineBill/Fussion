#pragma once

namespace Fussion {
    class ReflectionRegistry {
    public:
        static void register_data();

    private:
        static void register_generated();
    };
}

#define REGISTER_ENUM(QualifiedName, EnumName)                                          \
    {                                                                                   \
        auto ee = meta::enum_<QualifiedName>(meta::metadata_()("Name"s, #EnumName##s)); \
        for (auto const& [value, name] : magic_enum::enum_entries<QualifiedName>()) {   \
            ee.evalue_(std::string { name }, value);                                    \
        }                                                                               \
    }
