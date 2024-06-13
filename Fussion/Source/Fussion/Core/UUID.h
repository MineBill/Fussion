#pragma once
#include "Types.h"
#include "Reflect/Reflect.h"

namespace Fussion
{
    REFLECT_CLASS(REFLECT_LOOKUP_ONLY)
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
struct std::hash<Fussion::UUID>
{
    std::size_t operator()(const Fussion::UUID& id) const noexcept
    {
        using std::hash;
        return hash<u64>()(id);
    }
};

namespace Fsn = Fussion;