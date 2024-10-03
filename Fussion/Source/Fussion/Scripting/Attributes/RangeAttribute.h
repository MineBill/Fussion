#pragma once

#include <Fussion/Core/Types.h>
#include <Fussion/Scripting/Attribute.h>

namespace Fussion::Scripting {
    class RangeAttribute final : public Attribute {
    public:
        RangeAttribute(f32 min, f32 max, f32 step = 1.0f): m_min(min), m_max(max), m_step(step) {}

        [[nodiscard]]
        auto min() const -> f32 { return m_min; }

        [[nodiscard]]
        auto max() const -> f32 { return m_max; }

        [[nodiscard]]
        auto step() const -> f32 { return m_step; }

        virtual std::string ToString() override
        {
            return std::format("RangeAttribute({}, {}, {})", m_min, m_max, m_step);
        }

    private:
        f32 m_min{};
        f32 m_max{};
        f32 m_step{};
    };
}
