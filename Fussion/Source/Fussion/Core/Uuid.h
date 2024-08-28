#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Log/Formatters.h"

namespace Fussion {
    class Uuid {
    public:
        Uuid();
        explicit Uuid(u64 id) : m_Id(id) {}

        operator u64() const
        {
            return m_Id;
        }

        bool IsValid() const { return m_Id == 0; }

        static Uuid Invalid;
    private:
        u64 m_Id{};
    };
}

template<>
struct std::hash<Fussion::Uuid> {
    std::size_t operator()(const Fussion::Uuid& id) const noexcept
    {
        using std::hash;
        return hash<u64>()(id);
    }
};

namespace Fsn = Fussion;

FSN_MAKE_FORMATTABLE(Fussion::Uuid, "{}", static_cast<u64>(v))
