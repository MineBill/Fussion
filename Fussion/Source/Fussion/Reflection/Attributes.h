#pragma once

namespace Fussion::Attributes {
    class [[Attribute]] Region {
    public:
        std::string name{};
    };

    class [[Attribute]] Range {
    public:
        f32 min{};
        f32 max{};
        f32 step{ 1.0f };
    };

    class [[Attribute]] EditorName {
    public:
        std::string name{};
    };
}
