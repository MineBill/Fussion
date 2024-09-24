#pragma once

namespace Fussion::Attributes {
    struct [[Attribute]] RegionAttribute {
        std::string name{};
    };

    struct [[Attribute]] RangeAttribute {
        f32 min{};
        f32 max{};
        f32 step{ 1.0f };
    };

    struct [[Attribute]] EditorNameAttribute {
        std::string name{};
    };
}
