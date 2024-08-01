#include "e5pch.h"
#include "Ref.h"

namespace Fussion {
    void RefCounted::AddRef() { ++m_RefCount; }

    void RefCounted::Release()
    {
        if (--m_RefCount == 0)
            delete this;
    }

    u32 RefCounted::GetRefCount() const { return m_RefCount; }
}
