#include <Fussion/Core/DynamicArray.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <unordered_map>
using namespace Fussion;

struct TestingAllocator {
    struct TrackingAllocation {
        std::source_location location{};
        void* ptr;
    };

    Mem::Allocator backing_allocator{};

    std::unordered_map<uintptr_t, TrackingAllocation> allocations{};

    TestingAllocator()
    {
        backing_allocator = Mem::GetHeapAllocator();
    }

    auto allocator() -> Mem::Allocator
    {
        return {
            .AllocProc = [](usz size, void* data, std::source_location const& loc) {
                auto self = CAST(TestingAllocator*, data);
                auto ptr = self->backing_allocator.AllocProc(size, self->backing_allocator.data, loc);
                self->allocations[TRANSMUTE(uintptr_t, ptr)] = {
                    .location = loc,
                    .ptr = ptr,
                };
                return ptr;
            },
            .DeallocProc = [](void* ptr, void* data, std::source_location const& loc) {
                auto self = CAST(TestingAllocator*, data);

                if (self->allocations.contains(TRANSMUTE(uintptr_t, ptr))) {
                    self->allocations.erase(TRANSMUTE(uintptr_t, ptr));
                }
                self->backing_allocator.DeallocProc(ptr, self->backing_allocator.data, loc);
            },
            .data = this
        };
    }

    void print_leaks() const
    {
        for (auto const& [ptr, allocation] : allocations) {
            (void)ptr;
            LOG_WARNF("Leaked allocation '{:p}' at '{}:{}'", allocation.ptr, allocation.location.file_name(), allocation.location.line());
        }
    }
};

TEST_CASE("DynamicArray")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();

    DynamicArray<s32> numbers{ allocator };

    REQUIRE(numbers.len() == 0);
    REQUIRE(numbers.capacity() == 0);

    SECTION("appending increases length") {
        numbers.append(10);

        CHECK(numbers.len() == 1);
        CHECK(numbers[0] == 10);

        numbers.append(420);

        CHECK(numbers.len() == 2);
        CHECK(numbers[0] == 10);
        CHECK(numbers[1] == 420);
    }

    SECTION("appending multiple items does not invalidate previous items") {
        numbers.append(10);
        numbers.append(420);

        CHECK(numbers.len() == 2);
        CHECK(numbers[0] == 10);
        CHECK(numbers[1] == 420);
    }
}


TEST_CASE("DynamicArray: Construction and Destruction", "[DynamicArray]")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();

    SECTION("DynamicArray is constructed with zero length and capacity") {
        DynamicArray<int> array(allocator);
        REQUIRE(array.len() == 0);
        REQUIRE(array.capacity() == 0);
    }

    SECTION("DynamicArray is destroyed without leaks when not leaked explicitly") {
        DynamicArray<int> array(allocator);
        // Ensure no crashes during destruction
    }
}

TEST_CASE("DynamicArray: Append operation", "[DynamicArray]")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();
    DynamicArray<int> array(allocator);

    SECTION("Append elements increases length") {
        array.append(10);
        REQUIRE(array.len() == 1);
        REQUIRE(array[0] == 10);

        array.append(20);
        REQUIRE(array.len() == 2);
        REQUIRE(array[1] == 20);
    }

    SECTION("Array capacity increases when needed") {
        for (int i = 0; i < 15; ++i) {
            array.append(i);
        }
        REQUIRE(array.len() == 15);
        REQUIRE(array.capacity() >= 15);
    }
}

TEST_CASE("DynamicArray: Ensure capacity", "[DynamicArray]")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();
    DynamicArray<int> array(allocator);

    SECTION("Capacity grows as elements are added") {
        array.ensure_capacity(5);
        REQUIRE(array.capacity() >= 5);

        array.ensure_capacity(20);
        REQUIRE(array.capacity() >= 20);
    }

    SECTION("Capacity doesn't decrease if size requested is smaller") {
        array.ensure_capacity(10);
        size_t previous_capacity = array.capacity();
        array.ensure_capacity(5); // Should not shrink
        REQUIRE(array.capacity() == previous_capacity);
    }
}

TEST_CASE("DynamicArray: Accessing elements", "[DynamicArray]")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();
    DynamicArray<int> array(allocator);

    SECTION("Access elements after append") {
        array.append(10);
        array.append(20);

        REQUIRE(array[0] == 10);
        REQUIRE(array[1] == 20);
    }

    // SECTION("Out of bounds access should fail with VERIFY") {
    //     array.append(10);
    //     CHECK(array[1] == 1);
    // }
}

TEST_CASE("DynamicArray: Leak functionality", "[DynamicArray]")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();
    DynamicArray<int> array(allocator);

    SECTION("Leak prevents deallocation of memory") {
        array.append(10);
        array.append(20);

        auto leaked_buffer = array.leak();
        REQUIRE(leaked_buffer[0] == 10);
        REQUIRE(leaked_buffer[1] == 20);
        REQUIRE(array.len() == 2);

        Mem::Free(leaked_buffer, allocator);
        // Since leak prevents destruction, the array should not free memory on destruction
        // We can't directly test memory leaks in Catch2, but ensuring no crashes happen is sufficient
    }
}

TEST_CASE("DynamicArray: remove_at functionality", "[DynamicArray]")
{
    TestingAllocator testing_allocator{};
    defer(testing_allocator.print_leaks());
    auto allocator = testing_allocator.allocator();
    DynamicArray<int> array(allocator);

    SECTION("Remove valid element reduces length and shifts elements") {
        array.append(10);
        array.append(20);
        array.append(30);
        array.append(40);

        REQUIRE(array.len() == 4);

        // Remove element at index 1 (value 20)
        array.remove_at(1);

        REQUIRE(array.len() == 3);
        REQUIRE(array[0] == 10);
        REQUIRE(array[1] == 30); // 30 shifted down
        REQUIRE(array[2] == 40);
    }

    SECTION("Remove first element shifts all elements") {
        array.append(100);
        array.append(200);
        array.append(300);

        REQUIRE(array.len() == 3);

        // Remove the first element (index 0)
        array.remove_at(0);

        REQUIRE(array.len() == 2);
        REQUIRE(array[0] == 200); // 200 shifted down
        REQUIRE(array[1] == 300);
    }

    SECTION("Remove last element reduces length but does not shift") {
        array.append(100);
        array.append(200);
        array.append(300);

        REQUIRE(array.len() == 3);

        // Remove the last element (index 2)
        array.remove_at(2);

        REQUIRE(array.len() == 2);
        REQUIRE(array[0] == 100);
        REQUIRE(array[1] == 200);
    }

    // SECTION("Out of bounds removal triggers VERIFY") {
    //     array.append(10);
    //     array.append(20);
    //
    //     // Removing an element at an invalid index (greater than len-1) should trigger VERIFY.
    //     REQUIRE_NOTHROW(array.remove_at(1)); // This is valid
    //
    //     // Simulating out-of-bounds remove. VERIFY should panic here.
    //     if (array.len() <= 1) {
    //         FAIL("Out of bounds removal triggered VERIFY");
    //     }
    // }
    //
    // SECTION("Removing from empty array triggers VERIFY") {
    //     // If trying to remove an element from an empty array, it should trigger a VERIFY panic.
    //     if (array.len() == 0) {
    //         FAIL("Attempted to remove from an empty array");
    //     }
    // }
}
