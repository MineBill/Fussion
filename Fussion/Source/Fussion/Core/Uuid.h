#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Log/Formatters.h"

namespace Fussion {
    class Uuid {
    public:
        Uuid();
        explicit Uuid(u64 id)
            : m_ID(id)
        { }

        operator u64() const
        {
            return m_ID;
        }

        bool IsValid() const { return m_ID == 0; }

        static Uuid Invalid;

    private:
        u64 m_ID {};
    };
}

template<>
struct std::hash<Fussion::Uuid> {
    std::size_t operator()(Fussion::Uuid const& id) const noexcept
    {
        using std::hash;
        return hash<u64>()(id);
    }
};

namespace Fsn = Fussion;

FSN_MAKE_FORMATTABLE(Fussion::Uuid, "{}", static_cast<u64>(v))
