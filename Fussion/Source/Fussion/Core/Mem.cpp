#include "FussionPCH.h"
#include "Mem.h"

namespace Fussion::mem {

    bool is_power_of_two(size_t x)
    {
        return x && !(x & (x - 1));
    }

    thread_local Allocator HEAP_ALLOCATOR = {
        .alloc_proc = [](usz size, void*, std::source_location const&) {
            return ::malloc(size);
        },
        .dealloc_proc = [](void* ptr, void*, std::source_location const&) {
            ::free(ptr);
        },
        .data = nullptr
    };

    auto heap_allocator() -> Allocator
    {
        return HEAP_ALLOCATOR;
    }

    uintptr_t align_forward(uintptr_t ptr, usz alignment)
    {
        VERIFY(is_power_of_two(alignment));
        auto a = CAST(uintptr_t, alignment);
        if (auto mod = ptr & (a - 1)) {
            return ptr + a - mod;
        }
        return ptr;
    }

    void copy(void* dst, void const* src, usz length)
    {
        std::memcpy(dst, src, length);
    }

    s32 compare(void const* first, void const* second, usz length)
    {
        return std::memcmp(first, second, length);
    }
}
