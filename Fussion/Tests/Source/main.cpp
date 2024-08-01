#include "Common.h"

#define FSN_CORE_USE_GLOBALLY
#include <Fussion/Core/Uuid.h>
#include <Fussion/Core/SmallVector.h>
#include <Fussion/Core/Clap.h>
#include <Fussion/Core/Ref.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_session.hpp>

#include <cstdint>

TEST_CASE("SmallVector")
{
    Fussion::SmallVector<int, 10> vec{};

    SECTION("Push Back") {
        REQUIRE(vec.Size() == 0);

        REQUIRE(vec.PushBack(1).IsValue());
        REQUIRE(vec.PushBack(2).IsValue());

        REQUIRE(vec.Size() == 2);
    }

    SECTION("Pop Back") {
        REQUIRE(vec.Size() == 0);

        REQUIRE(vec.PushBack(1).IsValue());
        REQUIRE(vec.PopBack() == 1);

        REQUIRE(vec.Size() == 0);
    }

    SECTION("Push Error") {
        Fussion::SmallVector<int, 1> smaller{};

        REQUIRE(smaller.PushBack(1).IsValue());

        auto result = smaller.PushBack(1);
        REQUIRE(result.IsError());
        REQUIRE(result.Error() == Fussion::SmallVectorError::CapacityExceeded);
    }
}

TEST_CASE("Clap")
{
    Fussion::Clap clap{};

    clap.Option<std::string>("Project");
    clap.Option<bool>("EnableTests");
    clap.Option<f32>("Number");
    clap.Option<bool>("MissingOption");
    clap.Option<bool>("MissingButDefault", false);
    clap.Option<f32>("MissingButDefault2", 3.14);

    SECTION("All Options") {
        clap.Reset(R"(-Project=C:/Projects/Project -EnableTests=true -Number=3.14 -MissingOption=false -MissingButDefault=true -MissingButDefault2=3.14)");
        clap.Parse();

        REQUIRE(clap.Get<std::string>("Project") == "C:/Projects/Project");
        REQUIRE(clap.Get<bool>("EnableTests") == true);
        REQUIRE(clap.Get<f32>("Number") == 3.14f);
        REQUIRE(clap.Get<bool>("MissingOption") == false);
        REQUIRE(clap.Get<bool>("MissingButDefault") == true);
        REQUIRE(clap.Get<f32>("MissingButDefault2") == 3.14f);
    }

    SECTION("No Options") {
        clap.Reset("");
        clap.Parse();

        REQUIRE(clap.Get<std::string>("Project") == std::nullopt);
        REQUIRE(clap.Get<bool>("EnableTests") == std::nullopt);
        REQUIRE(clap.Get<f32>("Number") == std::nullopt);
        REQUIRE(clap.Get<bool>("MissingOption") == std::nullopt);
        REQUIRE(clap.Get<bool>("MissingButDefault") == false);
        REQUIRE(clap.Get<f32>("MissingButDefault2") == 3.14f);
    }

    SECTION("Bool without explicit value") {
        clap.Reset("-EnableTests");
        clap.Parse();

        REQUIRE(clap.Get<bool>("EnableTests") == true);
    }

    SECTION("Paths") {
        SECTION("Windows Paths") {
            clap.Reset("-Project=W:/some/random/path");
            clap.Parse();

            auto project = clap.Get<std::string>("Project");
            REQUIRE(project.has_value());
            REQUIRE(*project == "W:/some/random/path");
        }

        SECTION("Unix Paths") {
            clap.Option<std::string>("Project2");
            clap.Reset("-Project=/root/home/some/random/path -Project2=../some/other/path");
            clap.Parse();

            auto project = clap.Get<std::string>("Project");
            REQUIRE(project.has_value());
            REQUIRE(*project == "/root/home/some/random/path");

            auto project2 = clap.Get<std::string>("Project2");
            REQUIRE(project2.has_value());
            REQUIRE(*project2 == "../some/other/path");
        }
    }
}

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

TEST_CASE("RefCounted")
{
    struct Person : RefCounted {
        std::string Name{};
        s32 Age{};

        Person() {}
        Person(std::string name, s32 age) : Name(std::move(name)), Age(age) {}
    };

    SECTION("Release") {
        auto ptr = MakeRefPtr<Person>();

        REQUIRE(ptr->GetRefCount() == 1);
        ptr->Release();
    }

    SECTION("AddRef") {
        auto ptr = MakeRefPtr<Person>();
        ptr->AddRef();
        REQUIRE(ptr->GetRefCount() == 2);
    }

    SECTION("Multiple References") {
        auto ptr = MakeRefPtr<Person>();
        auto ptr2 = ptr;

        REQUIRE(ptr->GetRefCount() == 2);
        REQUIRE(ptr2->GetRefCount() == 2);

        REQUIRE(ptr->Age == ptr2->Age);
        REQUIRE(ptr->Name == ptr2->Name);
    }

    SECTION("Return Reference") {
        auto ReturnsAReference = []() -> RefPtr<Person> {
            return MakeRefPtr<Person>("Bob", 42);
        };

        auto ptr = ReturnsAReference();
        REQUIRE(ptr->GetRefCount() == 1);
        REQUIRE(ptr->Age == 42);
    }
}

TEST_CASE("UUID", "[!benchmark]")
{
    constexpr auto iter_count = 100000;
    BENCHMARK("UUID") {
        for (u32 i = 0; i < iter_count; ++i) {
            (void)Fussion::Uuid();
        }
    };

    BENCHMARK("uuidv7") {
        for (u32 i = 0; i < iter_count; ++i) {
            (void)uuidv7();
        }
    };
}

int main(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
