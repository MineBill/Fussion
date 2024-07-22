#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Log/Formatters.h"

namespace Fussion
{
    class Uuid
    {
    public:
        Uuid();
        Uuid(const u64 id) : m_Id(id) {}

        operator u64() const
        {
            return m_Id;
        }

    private:
        u64 m_Id{};
    };
}

template <>
struct std::hash<Fussion::Uuid>
{
    std::size_t operator()(const Fussion::Uuid& id) const noexcept
    {
        using std::hash;
        return hash<u64>()(id);
    }
};

namespace Fsn = Fussion;

FSN_MAKE_FORMATTABLE(Fussion::Uuid, "{}", static_cast<u64>(v))