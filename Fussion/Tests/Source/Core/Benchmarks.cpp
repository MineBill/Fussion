#include "Fussion/Core/Uuid.h"

#include <Fussion/Core/Ref.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <unordered_map>
#include <ranges>

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
    BENCHMARK("UUID") {
        return Uuid();
    };

    BENCHMARK("uuidv7") {
        return uuidv7();
    };
}

TEST_CASE("Iteration", "[!benchmark]")
{
    struct MyStruct {
        std::string Name{};
        bool DoThing{};
        s32 Number{};

        s32 DoOtherThing() const
        {
            return Number * 22 + Name.size();
        }
    };

    std::unordered_map<s32, MyStruct> data{};

    for (s32 i = 0; i < 1000; i++) {
        data[i] = { "John", false, i + 10 };
    }

    BENCHMARK("std::views::values") {
        s32 result = 0;
        for (auto const& s : data | std::views::values) {
            result += s.DoOtherThing();
        }

        return result;
    };

    BENCHMARK("(void)key") {
        s32 result = 0;
        for (auto const& [id, s] : data) {
            (void)id;
            result += s.DoOtherThing();
        }

        return result;
    };
}
