#include "Fussion/Core/Maybe.h"
#include "Fussion/Core/StringUtils.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Fussion/Core/SmallVector.h>
#include <Fussion/Core/Clap.h>
#include <Fussion/Core/Ref.h>

using namespace Fussion;

TEST_CASE("SmallVector")
{
    SmallVector<int, 10> vec{};

    SECTION("Push Back") {
        REQUIRE(vec.Size() == 0);

        CHECK(vec.PushBack(1).IsValue());
        CHECK(vec.PushBack(2).IsValue());

        CHECK(vec.Size() == 2);
    }

    SECTION("Pop Back") {
        REQUIRE(vec.Size() == 0);

        CHECK(vec.PushBack(1).IsValue());
        CHECK(vec.PopBack() == 1);

        CHECK(vec.Size() == 0);
    }

    SECTION("Push Error") {
        SmallVector<int, 1> smaller{};

        CHECK(smaller.PushBack(1).IsValue());

        auto result = smaller.PushBack(1);
        CHECK(result.IsError());
        CHECK(result.Error() == Fussion::SmallVectorError::CapacityExceeded);
    }
}

TEST_CASE("Clap")
{
    Clap clap{};

    clap.Option<std::string>("Project");
    clap.Option<bool>("EnableTests");
    clap.Option<f32>("Number");
    clap.Option<bool>("MissingOption");
    clap.Option<bool>("MissingButDefault", false);
    clap.Option<f32>("MissingButDefault2", 3.14);

    SECTION("All Options") {
        clap.Reset(R"(-Project=C:/Projects/Project -EnableTests=true -Number=3.14 -MissingOption=false -MissingButDefault=true -MissingButDefault2=3.14)");
        clap.Parse();

        CHECK(clap.Get<std::string>("Project") == "C:/Projects/Project");
        CHECK(clap.Get<bool>("EnableTests") == true);
        CHECK(clap.Get<f32>("Number") == 3.14f);
        CHECK(clap.Get<bool>("MissingOption") == false);
        CHECK(clap.Get<bool>("MissingButDefault") == true);
        CHECK(clap.Get<f32>("MissingButDefault2") == 3.14f);
    }

    SECTION("No Options") {
        clap.Reset("");
        clap.Parse();

        CHECK(clap.Get<std::string>("Project") == std::nullopt);
        CHECK(clap.Get<bool>("EnableTests") == std::nullopt);
        CHECK(clap.Get<f32>("Number") == std::nullopt);
        CHECK(clap.Get<bool>("MissingOption") == std::nullopt);
        CHECK(clap.Get<bool>("MissingButDefault") == false);
        CHECK(clap.Get<f32>("MissingButDefault2") == 3.14f);
    }

    SECTION("Bool without explicit value") {
        clap.Reset("-EnableTests");
        clap.Parse();

        CHECK(clap.Get<bool>("EnableTests") == true);
    }

    SECTION("Paths") {
        SECTION("Windows Paths") {
            clap.Reset("-Project=W:/some/random/path");
            clap.Parse();

            auto project = clap.Get<std::string>("Project");
            CHECK(project.has_value());
            CHECK(*project == "W:/some/random/path");
        }

        SECTION("Unix Paths") {
            clap.Option<std::string>("Project2");
            clap.Reset("-Project=/root/home/some/random/path -Project2=../some/other/path");
            clap.Parse();

            auto project = clap.Get<std::string>("Project");
            CHECK(project.has_value());
            CHECK(*project == "/root/home/some/random/path");

            auto project2 = clap.Get<std::string>("Project2");
            CHECK(project2.has_value());
            CHECK(*project2 == "../some/other/path");
        }
    }
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

        CHECK(ptr->GetRefCount() == 1);
        ptr->Release();
    }

    SECTION("AddRef") {
        auto ptr = MakeRefPtr<Person>();
        ptr->AddRef();
        CHECK(ptr->GetRefCount() == 2);
    }

    SECTION("Multiple References") {
        auto ptr = MakeRefPtr<Person>();
        auto ptr2 = ptr;

        CHECK(ptr->GetRefCount() == 2);
        CHECK(ptr2->GetRefCount() == 2);

        CHECK(ptr->Age == ptr2->Age);
        CHECK(ptr->Name == ptr2->Name);
    }

    SECTION("Return Reference") {
        auto ReturnsAReference = []() -> RefPtr<Person> {
            return MakeRefPtr<Person>("Bob", 42);
        };

        auto ptr = ReturnsAReference();
        CHECK(ptr->GetRefCount() == 1);
        CHECK(ptr->Age == 42);
    }
}

struct DebugObject {
    DebugObject()
    {
        LOG_DEBUGF("Constructed");
    }

    ~DebugObject()
    {
        LOG_DEBUGF("Destructed. Moved: {}", Moved);

    }

    DebugObject(DebugObject const& other)
    {
        LOG_DEBUGF("Copied");

    }

    DebugObject(DebugObject&& other) noexcept
    {
        other.Moved = true;
        LOG_DEBUGF("Moved");
    }

    DebugObject& operator=(DebugObject const& other)
    {
        if (this == &other)
            return *this;
        LOG_DEBUGF("Assigned");
        return *this;
    }

    DebugObject& operator=(DebugObject&& other) noexcept
    {
        if (this == &other)
            return *this;
        LOG_DEBUGF("Assigned-Moved?");
        return *this;
    }

    bool Moved{ false };
};

TEST_CASE("Optional")
{
    SECTION("Trivial Types") {
        SECTION("Assignment") {
            Maybe<int> num;

            CHECK(num.IsEmpty());

            num = 2;
            CHECK(num.HasValue());
            CHECK(num == 2);
        }

        SECTION("Copy") {
            Maybe<int> first;

            CHECK(first.IsEmpty());

            Maybe<int> second;
            first = second;

            CHECK(first.IsEmpty());
            CHECK(second.IsEmpty());

            Maybe<int> third = 33;

            first = third;
            second = first;

            CHECK(first.HasValue());
            CHECK(first == 33);

            CHECK(first == second);
        }
    }

    SECTION("Objects") {
        SECTION("Assignment") {
            Maybe<DebugObject> num;

            CHECK(num.IsEmpty());

            num = DebugObject();
            CHECK(num.HasValue());
        }
    }
}

TEST_CASE("StringUtils")
{
    using namespace std::string_view_literals;
    SECTION("::Remove") {
        CHECK(StringUtils::Remove("m_Enabled", "m_") == "Enabled"sv);
        CHECK(StringUtils::Remove("Enabled", "m_") == "Enabled"sv);
    }
}
