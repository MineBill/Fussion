#pragma once

namespace Fussion::Attributes {
    struct [[Attribute]] RegionAttribute {
        std::string Name {};
    };

    struct [[Attribute]] RangeAttribute {
        f64 Min {};
        f64 Max {};
        f64 Step { 1.0f };
    };

    struct [[Attribute]] EditorNameAttribute {
        std::string Name {};
    };

    struct [[Attribute]] EditorButtonAttribute {
        std::string ButtonText {};
    };

    struct [[Attribute]] NotifyForAttribute {
        std::string MemberName {};
    };
}
