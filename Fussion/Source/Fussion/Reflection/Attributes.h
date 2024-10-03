#pragma once

namespace Fussion::Attributes {
    struct [[Attribute]] RegionAttribute {
        std::string Name {};
    };

    struct [[Attribute]] RangeAttribute {
        f32 Min {};
        f32 Max {};
        f32 Step { 1.0f };
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
