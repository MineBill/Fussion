#include "Fussion/Core/String.h"
#include <Fussion/Core/Mem.h>

#include <iostream>
#include <unordered_map>

using namespace Fussion;

struct TrackingAllocator {
    mem::Allocator backing_allocator;
    std::unordered_map<void*, s32> allocations;

    auto allocator() -> mem::Allocator
    {
        return {
            .alloc_proc = [](usz size, void* data, std::source_location const& loc) -> void* {
                auto self = CAST(TrackingAllocator*, data);
                auto ptr = self->backing_allocator.alloc_proc(size, self->backing_allocator.data, loc);
                self->allocations.emplace(ptr, size);
                return ptr;
            },
            .dealloc_proc = [](void* ptr, void* data, std::source_location const& loc) {
                auto self = CAST(TrackingAllocator*, data);
                self->allocations.erase(ptr);
                self->backing_allocator.dealloc_proc(ptr, self->backing_allocator.data, loc);
            },
            .data = this
        };
    }

    void check()
    {
        if (!allocations.empty()) {
            std::cout << "Leak!" << std::endl;
        }
    }
};

struct ArenaAllocator {
    auto allocator() -> mem::Allocator
    {
        return {
            .alloc_proc = [](usz size, void* data, std::source_location const& loc) -> void* {
                auto self = CAST(ArenaAllocator*, data);
                if ((self->m_offset + size) > self->m_capacity) {
                    LOG_ERRORF("{} {}", self->m_offset, size);
                }
                auto ptr = mem::align_forward(TRANSMUTE(uintptr_t, self->m_base_ptr) + self->m_offset, mem::DEFAULT_ALIGNMENT);
                // Alignment might change the offset so we can't just += size.
                self->m_offset = (ptr - TRANSMUTE(uintptr_t, self->m_base_ptr)) + size;
                return TRANSMUTE(void*, ptr);
            },
            .dealloc_proc = [](void* ptr, void* data, std::source_location const& loc) {
                (void)ptr;
                (void)data;
            },
            .data = this
        };
    }

    static ArenaAllocator create(mem::Allocator const& backing, usz initial_capacity)
    {
        ArenaAllocator allocator;
        allocator.m_backing_allocator = backing;
        allocator.m_capacity = initial_capacity;
        allocator.m_base_ptr = mem::alloc(initial_capacity, backing);
        return allocator;
    }

private:
    mem::Allocator m_backing_allocator{};
    usz m_capacity{};
    void* m_base_ptr{};
    usz m_offset{};
};

int main()
{
    auto arena = ArenaAllocator::create(mem::heap_allocator(), 1'000 * 10);

    auto allocator = arena.allocator();
    Slice<u8> bytes = mem::alloc<u8>(10, allocator);
    s32* number = mem::alloc<s32>(allocator);
    *number = 12;
    auto* big_number = mem::alloc<s64>(allocator);
    *big_number = 11;

    std::cout << *number << std::endl;
    *number = 9;
    std::cout << *big_number << std::endl;

    String s("Hello World!");

    std::cout << "sizeof(void*): " << sizeof(void*) << std::endl;
    for (usz i = 0; i < bytes.length; ++i) {
        bytes[i] = 'A' + i;
        std::cout << bytes[i] << std::endl;
    }
    String ss = String::alloc("Jonathan");
    ss.free();
    auto asd = mem::alloc<u8>(10);
    mem::free(asd);
}
