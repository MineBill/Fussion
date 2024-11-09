#pragma once
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Span.h>

#include <source_location>

namespace Fussion::Mem {
    /// This is kind of a random value. Taken from Odin and C3.
    constexpr auto DEFAULT_ALIGNMENT = sizeof(void*) * 2;

    struct Allocator {
        using AllocationProc = void* (*)(usz size, void*, std::source_location const&);
        using DeallocationProc = void (*)(void* ptr, void*, std::source_location const&);

        AllocationProc alloc_proc {
            [](usz, void*, std::source_location const&) -> void* {
                PANIC("Tried to allocate memory from an uninitialized allocators");
            }
        };
        DeallocationProc dealloc_proc {
            [](void*, void*, std::source_location const&) {
                PANIC("Tried to deallocate from an uninitialized allocator");
            }
        };

        void* data;
    };

    auto heap_allocator() -> Allocator;
    auto temp_allocator() -> Allocator;

    uintptr_t align_forward(uintptr_t ptr, size_t alignment);

    inline void* alloc(
        usz size,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current()
    )
    {
        return allocator.alloc_proc(size, allocator.data, loc);
    }

    inline void free(
        void* ptr,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current()
    )
    {
        allocator.dealloc_proc(ptr, allocator.data, loc);
    }

    void copy(void* dst, void const* src, size_t length);
    s32 compare(void const* first, void const* second, size_t length);

    template<typename T>
    Span<T> alloc(
        usz size,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current()
    )
    {
        return Span<T>(CAST(T*, alloc(size * sizeof(T), allocator, loc)), size);
    }

    template<typename T>
    T* alloc(Allocator const& allocator, std::source_location const& loc = std::source_location::current())
    {
        return CAST(T*, alloc(sizeof(T), allocator, loc));
    }

    template<typename T>
    void free(
        Span<T>& span,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current()
    )
    {
        allocator.dealloc_proc(span.data(), allocator.data, loc);
        span.reset();
    }

    template<typename T>
    void copy(Span<T> const& dst, Span<T> const& src)
    {
        VERIFY(dst.size() >= src.size(), "dst: {}, src: {}", dst.size(), src.size());
        Mem::copy(dst.data(), src.data(), src.size() * sizeof(T));
    }

    template<typename T>
    s32 compare(Span<T> const& first, Span<T> const& second)
    {
        if (first.size() < second.size()) {
            return -1;
        }
        if (first.size() > second.size()) {
            return 1;
        }
        return compare(first.data(), second.data(), first.size());
    }
}
