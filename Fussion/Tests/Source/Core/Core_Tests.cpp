#include "../Common.h"

#include "Fussion/Core/Maybe.h"
#include "Fussion/Core/StringUtils.h"
#include <Fussion/Core/SmallVector.h>
#include <Fussion/Core/Ref.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Fussion;

TEST_CASE("SmallVector")
{
    SmallVector<int, 10> vec{};

    SECTION("Push Back") {
        REQUIRE(vec.size() == 0);

        CHECK(vec.push_back(1).is_value());
        CHECK(vec.push_back(2).is_value());

        CHECK(vec.size() == 2);
    }

    SECTION("Pop Back") {
        REQUIRE(vec.size() == 0);

        CHECK(vec.push_back(1).is_value());
        CHECK(vec.pop_back() == 1);

        CHECK(vec.size() == 0);
    }

    SECTION("Push Error") {
        SmallVector<int, 1> smaller{};

        CHECK(smaller.push_back(1).is_value());

        auto result = smaller.push_back(1);
        CHECK(result.is_error());
        CHECK(result.error() == Fussion::SmallVectorError::CapacityExceeded);
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
        auto ptr = make_ref_ptr<Person>();

        CHECK(ptr->ref_count() == 1);
        ptr->release();
    }

    SECTION("AddRef") {
        auto ptr = make_ref_ptr<Person>();
        ptr->add_ref();
        CHECK(ptr->ref_count() == 2);
    }

    SECTION("Multiple References") {
        auto ptr = make_ref_ptr<Person>();
        auto ptr2 = ptr;

        CHECK(ptr->ref_count() == 2);
        CHECK(ptr2->ref_count() == 2);

        CHECK(ptr->Age == ptr2->Age);
        CHECK(ptr->Name == ptr2->Name);
    }

    SECTION("Return Reference") {
        auto ReturnsAReference = []() -> RefPtr<Person> {
            return make_ref_ptr<Person>("Bob", 42);
        };

        auto ptr = ReturnsAReference();
        CHECK(ptr->ref_count() == 1);
        CHECK(ptr->Age == 42);
    }
}

TEST_CASE("Optional")
{
    SECTION("Trivial Types") {
        SECTION("Assignment") {
            Maybe<int> num;

            CHECK(num.is_empty());

            num = 2;
            CHECK(num.has_value());
            CHECK(num == 2);
        }

        SECTION("Copy") {
            Maybe<int> first;

            CHECK(first.is_empty());

            Maybe<int> second;
            first = second;

            CHECK(first.is_empty());
            CHECK(second.is_empty());

            Maybe<int> third = 33;

            first = third;
            second = first;

            CHECK(first.has_value());
            CHECK(first == 33);

            CHECK(first == second);
        }
    }

    SECTION("Objects") {
        SECTION("Assignment") {
            Maybe<DebugObject> num;

            CHECK(num.is_empty());

            num = DebugObject();
            CHECK(num.has_value());
        }
    }
}

TEST_CASE("StringUtils")
{
    using namespace std::string_view_literals;
    SECTION("::Remove") {
        CHECK(StringUtils::remove("m_Enabled", "m_") == "Enabled"sv);
        CHECK(StringUtils::remove("Enabled", "m_") == "Enabled"sv);
    }
}
