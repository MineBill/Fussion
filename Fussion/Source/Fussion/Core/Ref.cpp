#include "FussionPCH.h"
#include "Ref.h"

namespace Fussion {
    void RefCounted::AddRef() { ++m_ref_count; }

    void RefCounted::Release()
    {
        if (--m_ref_count == 0)
            delete this;
    }

    u32 RefCounted::RefCount() const { return m_ref_count; }
}
