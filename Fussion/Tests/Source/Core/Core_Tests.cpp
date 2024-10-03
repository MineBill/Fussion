#include "../Common.h"

#include "Fussion/Core/Maybe.h"
#include "Fussion/Core/StringUtils.h"
#include <Fussion/Core/Ref.h>
#include <Fussion/Core/SmallVector.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Fussion;

TEST_CASE("SmallVector")
{
    SmallVector<int, 10> vec {};

    SECTION("Push Back")
    {
        REQUIRE(vec.size() == 0);

        CHECK(vec.Append(1).IsValue());
        CHECK(vec.Append(2).IsValue());

        CHECK(vec.size() == 2);
    }

    SECTION("Pop Back")
    {
        REQUIRE(vec.size() == 0);

        CHECK(vec.Append(1).IsValue());
        CHECK(vec.Pop() == 1);

        CHECK(vec.size() == 0);
    }

    SECTION("Push Error")
    {
        SmallVector<int, 1> smaller {};

        CHECK(smaller.Append(1).IsValue());

        auto result = smaller.Append(1);
        CHECK(result.HasError());
        CHECK(result.Error() == Fussion::SmallVectorError::CapacityExceeded);
    }
}

TEST_CASE("RefCounted")
{
    struct Person : RefCounted {
        std::string Name {};
        s32 Age {};

        Person() { }
        Person(std::string name, s32 age)
            : Name(std::move(name))
            , Age(age)
        { }
    };

    SECTION("Release")
    {
        auto ptr = MakeRefPtr<Person>();

        CHECK(ptr->RefCount() == 1);
        ptr->Release();
    }

    SECTION("AddRef")
    {
        auto ptr = MakeRefPtr<Person>();
        ptr->AddRef();
        CHECK(ptr->RefCount() == 2);
    }

    SECTION("Multiple References")
    {
        auto ptr = MakeRefPtr<Person>();
        auto ptr2 = ptr;

        CHECK(ptr->RefCount() == 2);
        CHECK(ptr2->RefCount() == 2);

        CHECK(ptr->Age == ptr2->Age);
        CHECK(ptr->Name == ptr2->Name);
    }

    SECTION("Return Reference")
    {
        auto ReturnsAReference = []() -> RefPtr<Person> {
            return MakeRefPtr<Person>("Bob", 42);
        };

        auto ptr = ReturnsAReference();
        CHECK(ptr->RefCount() == 1);
        CHECK(ptr->Age == 42);
    }
}

TEST_CASE("Optional")
{
    SECTION("Trivial Types")
    {
        SECTION("Assignment")
        {
            Maybe<int> num;

            CHECK(num.IsEmpty());

            num = 2;
            CHECK(num.HasValue());
            CHECK(num == 2);
        }

        SECTION("Copy")
        {
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

    SECTION("Objects")
    {
        SECTION("Assignment")
        {
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
    SECTION("::Remove")
    {
        CHECK(StringUtils::Remove("m_Enabled", "m_") == "Enabled"sv);
        CHECK(StringUtils::Remove("Enabled", "m_") == "Enabled"sv);
    }
}
