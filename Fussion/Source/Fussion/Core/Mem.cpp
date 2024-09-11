#include "FussionPCH.h"
#include "Mem.h"

namespace Fussion::mem {
    struct TempAllocator {
        TempAllocator()
        {
            m_buffer = mem::alloc<u8>(1'000'000, mem::heap_allocator());
        }

        auto allocator() -> Allocator
        {
            return {
                .alloc_proc = [](usz size, void* data, std::source_location const&) -> void* {
                    auto self = TRANSMUTE(TempAllocator*, data);
                    if (self->m_offset + size >= self->m_buffer.length) {
                        self->m_offset = 0;
                    }
                    auto ptr = self->m_buffer.ptr + self->m_offset;
                    self->m_offset += size;
                    return ptr;
                },
                .dealloc_proc = [](void*, void*, std::source_location const&) {},
                .data = this,
            };
        }

    private:
        Slice<u8> m_buffer{};
        usz m_offset{};
    };

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

    thread_local TempAllocator TEMP_ALLOCATOR_DATA{};
    thread_local Allocator TEMP_ALLOCATOR = TEMP_ALLOCATOR_DATA.allocator();

    auto heap_allocator() -> Allocator
    {
        return HEAP_ALLOCATOR;
    }

    auto temp_allocator() -> Allocator
    {
        return TEMP_ALLOCATOR;
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
