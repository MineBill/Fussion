#include "FussionPCH.h"
#include "Ref.h"

namespace Fussion {
    void RefCounted::add_ref() { ++m_ref_count; }

    void RefCounted::release()
    {
        if (--m_ref_count == 0)
            delete this;
    }

    u32 RefCounted::ref_count() const { return m_ref_count; }
}
