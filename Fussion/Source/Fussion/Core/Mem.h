#pragma once
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Slice.h>

#include <source_location>

namespace Fussion::Mem {
    /// This is kind of a random value. Taken from Odin and C3.
    constexpr auto DEFAULT_ALIGNMENT = sizeof(void*) * 2;

    struct Allocator {
        using AllocationProc = void* (*)(usz size, void*, std::source_location const&);
        using DeallocationProc = void (*)(void* ptr, void*, std::source_location const&);

        AllocationProc AllocProc {
            [](usz, void*, std::source_location const&) -> void* {
                PANIC("Tried to allocate memory from an uninitialized allocators");
            }
        };
        DeallocationProc DeallocProc {
            [](void*, void*, std::source_location const&) {
                PANIC("Tried to deallocate from an uninitialized allocator");
            }
        };

        void* data;
    };

    auto GetHeapAllocator() -> Allocator;
    auto GetTempAllocator() -> Allocator;

    uintptr_t AlignForward(uintptr_t ptr, size_t alignment);

    inline void* Alloc(
        usz size,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current())
    {
        return allocator.AllocProc(size, allocator.data, loc);
    }

    inline void Free(
        void* ptr,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current())
    {
        allocator.DeallocProc(ptr, allocator.data, loc);
    }

    void Copy(void* dst, void const* src, size_t length);
    s32 Compare(void const* first, void const* second, size_t length);

    template<typename T>
    Slice<T> Alloc(
        usz size,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current())
    {
        return Slice<T>(CAST(T*, Alloc(size * sizeof(T), allocator, loc)), size);
    }

    template<typename T>
    T* Alloc(Allocator const& allocator, std::source_location const& loc = std::source_location::current())
    {
        return CAST(T*, Alloc(sizeof(T), allocator, loc));
    }

    template<typename T>
    void Free(
        Slice<T>& slice,
        Allocator const& allocator,
        std::source_location const& loc = std::source_location::current())
    {
        slice.length = 0;
        allocator.DeallocProc(slice.ptr, allocator.data, loc);
    }

    template<typename T>
    void Copy(Slice<T> const& dst, Slice<T> const& src)
    {
        VERIFY(dst.length >= src.length, "dst: {}, src: {}", dst.length, src.length);
        Mem::Copy(dst.ptr, src.ptr, src.length * sizeof(T));
    }

    template<typename T>
    s32 Compare(Slice<T> const& first, Slice<T> const& second)
    {
        if (first.length < second.length) {
            return -1;
        }
        if (first.length > second.length) {
            return 1;
        }
        return Compare(first.ptr, second.ptr, first.length);
    }
}
