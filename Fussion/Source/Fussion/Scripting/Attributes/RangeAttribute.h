#pragma once

#include <Fussion/Core/Types.h>
#include <Fussion/Scripting/Attribute.h>

namespace Fussion::Scripting {
class RangeAttribute final : public Attribute {
public:
    RangeAttribute(f32 min, f32 max, f32 step = 1.0f): m_Min(min), m_Max(max), m_Step(step) {}

    [[nodiscard]]
    auto Min() const -> f32 { return m_Min; }

    [[nodiscard]]
    auto Max() const -> f32 { return m_Max; }

    [[nodiscard]]
    auto Step() const -> f32 { return m_Step; }

    virtual std::string ToString() override
    {
        return std::format("RangeAttribute({}, {}, {})", m_Min, m_Max, m_Step);
    }
private:
    f32 m_Min{};
    f32 m_Max{};
    f32 m_Step{};
};
}
