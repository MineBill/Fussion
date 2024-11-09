#pragma once

namespace Fussion::Attributes {
    struct [[Attribute]] RegionAttribute {
        std::string name {};
    };

    struct [[Attribute]] RangeAttribute {
        f64 min {};
        f64 max {};
        f64 step { 1.0f };
    };

    struct [[Attribute]] EditorNameAttribute {
        std::string name {};
    };

    struct [[Attribute]] EditorButtonAttribute {
        std::string button_text {};
    };

    struct [[Attribute]] NotifyForAttribute {
        std::string member_name {};
    };
}
