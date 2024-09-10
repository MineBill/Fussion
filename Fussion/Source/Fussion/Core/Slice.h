#pragma once
#include <Fussion/Core/Core.h>

namespace Fussion {
    template<typename T>
    struct Slice {
        T* ptr;
        usz length;

        T& operator[](usz index) const
        {
            VERIFY(index < length);
            return ptr[index];
        }
    };
}

#if FSN_CORE_USE_GLOBALLY
using Fussion::Slice;
#endif
