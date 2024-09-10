#pragma once
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Slice.h>

#include <source_location>

namespace Fussion::mem {
    /// This is kind of a random value. Taken from Odin and C3.
    constexpr auto DEFAULT_ALIGNMENT = sizeof(void*) * 2;

    struct Allocator {
        using AllocationProc = void*(*)(usz size, void*, std::source_location const&);
        using DeallocationProc = void(*)(void* ptr, void*, std::source_location const&);

        // Allocator()
        // {
        //     alloc_proc = [](usz, void*) -> void* {
        //         PANIC("Tried to allocate memory from an uninitialized allocators");
        //     };
        //
        //     dealloc_proc = [](void*, void*) {
        //         PANIC("Tried to deallocate from an uninitialized allocator");
        //     };
        //
        //     data = nullptr;
        // }

        AllocationProc alloc_proc{
            [](usz, void*, std::source_location const&) -> void* {
                PANIC("Tried to allocate memory from an uninitialized allocators");
            } };
        DeallocationProc dealloc_proc{
            [](void*, void*, std::source_location const&) {
                PANIC("Tried to deallocate from an uninitialized allocator");
            } };

        void* data;
    };

    auto heap_allocator() -> Allocator;

    uintptr_t align_forward(uintptr_t ptr, size_t alignment);

    inline void* alloc(
        usz size,
        Allocator const& allocator = heap_allocator(),
        std::source_location const& loc = std::source_location::current())
    {
        return allocator.alloc_proc(size, allocator.data, loc);
    }

    inline void free(
        void* ptr,
        Allocator const& allocator = heap_allocator(),
        std::source_location const& loc = std::source_location::current())
    {
        allocator.dealloc_proc(ptr, allocator.data, loc);
    }

    void copy(void* dst, void const* src, size_t length);
    s32 compare(void const* first, void const* second, size_t length);

    template<typename T>
    Slice<T> alloc(
        usz size,
        Allocator const& allocator = heap_allocator(),
        std::source_location const& loc = std::source_location::current())
    {
        return Slice<T>{
            .ptr = CAST(T*, alloc(size * sizeof(T), allocator, loc)),
            .length = size,
        };
    }

    template<typename T>
    T* alloc(Allocator const& allocator = heap_allocator(), std::source_location const& loc = std::source_location::current())
    {
        return CAST(T*, alloc(sizeof(T), allocator, loc));
    }

    template<typename T>
    void free(
        Slice<T> const& slice,
        Allocator const& allocator = heap_allocator(),
        std::source_location const& loc = std::source_location::current())
    {
        allocator.dealloc_proc(slice.ptr, allocator.data, loc);
    }

    template<typename T>
    void copy(Slice<T>& dst, Slice<T> const& src)
    {
        VERIFY(dst.length >= src.length);
        mem::copy(dst.ptr, src.ptr, src.length);
    }

    template<typename T>
    s32 compare(Slice<T> const& first, Slice<T> const& second)
    {
        if (first.length < second.length) {
            return -1;
        }
        if (first.length > second.length) {
            return 1;
        }
        return compare(first.ptr, second.ptr, first.length);
    }
}
