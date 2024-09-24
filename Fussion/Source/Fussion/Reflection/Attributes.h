#pragma once

namespace Fussion::Attributes {
    struct [[Attribute]] Region {
        std::string name{};
    };

    struct [[Attribute]] Range {
        f32 min{};
        f32 max{};
        f32 step{ 1.0f };
    };

    struct [[Attribute]] EditorName {
        std::string name{};
    };
}
