#include "Fussion/Core/Uuid.h"

#include <Fussion/Core/Ref.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

using namespace Fussion;

TEST_CASE("std::shared_ptr", "[!benchmark]")
{
    class SimpleClass {
    public:
        int X = 0;
        int Y = 0;
    };

    BENCHMARK("make_shared") {
        return std::make_shared<SimpleClass>();
    };

    BENCHMARK("copy") {
        auto ptr = std::make_shared<SimpleClass>();
        std::shared_ptr<SimpleClass> ptr2 = ptr;
        return ptr2;
    };
}

TEST_CASE("RefCounted Benchmarks", "[!benchmark]")
{
    class SimpleClass : public RefCounted {
    public:
        int X = 0;
        int Y = 0;
    };

    BENCHMARK("make") {
        return MakeRefPtr<SimpleClass>();
    };

    BENCHMARK("copy") {
        auto ptr = MakeRefPtr<SimpleClass>();
        RefPtr<SimpleClass> ptr2 = ptr;
        return ptr2;
    };

    BENCHMARK("Assignment") {
        auto ptr = MakeRefPtr<SimpleClass>();
        auto ptr2 = MakeRefPtr<SimpleClass>();
        ptr2 = ptr;
        return ptr2;
    };

}

#include "../Common.h"
TEST_CASE("UUID", "[!benchmark]")
{
    constexpr auto iter_count = 100000;
    BENCHMARK("UUID") {
        for (u32 i = 0; i < iter_count; ++i) {
            (void)Uuid();
        }
    };

    BENCHMARK("uuidv7") {
        for (u32 i = 0; i < iter_count; ++i) {
            (void)uuidv7();
        }
    };
}
