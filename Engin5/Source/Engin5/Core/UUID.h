#pragma once
#include "Types.h"

namespace Engin5
{
    class UUID
    {
    public:
        UUID();
        UUID(const u64 id) : m_Id(id) {}

        operator u64() const
        {
            return m_Id;
        }

    private:
        u64 m_Id{};
    };
}

template <>
struct std::hash<Engin5::UUID>
{
    std::size_t operator()(const Engin5::UUID& id) const noexcept
    {
        using std::hash;
        return hash<u64>()(id);
    }
};