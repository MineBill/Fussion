#include <iostream>
#include <Fussion/Core/String.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
using namespace Fussion;

namespace Catch {
    template<>
    struct StringMaker<String> {
        static std::string convert(String const& value) {
            return std::string(value.data.ptr, value.len());
        }
    };
}

TEST_CASE("String")
{
    SECTION("String literals") {
        String s = "Hello World";
        CHECK(s.len() == sizeof("Hello World") - 1);
    }

    SECTION("Alloc") {
        String s = String::alloc("AWDWAD");
        CHECK(s.len() == sizeof("AWDWAD") - 1);
        CHECK(s[0] == 'A');

        s[0] = 1;

        CHECK(s[0] == 1);
    }

    SECTION("Clone") {
        String s = String::alloc("Jonathan");
        String view = s.clone();
        s.free();
    }

    SECTION("index of") {
        String s = "This is a string";
        auto space = s.index_of(" ");
        CHECK(space.has_value());
        CHECK(space.value() == 4);

        CHECK(s.index_of("_").is_empty());

        auto is = s.index_of("is");
        CHECK(is.value() == 2);
    }

    SECTION("split") {
        {
            String s = "this_is_a_string";
            auto parts = s.split("_");
            CHECK(parts.length == 4);

            CHECK(parts[0] == "this");
            CHECK(parts[1] == "is");
            CHECK(parts[2] == "a");
            CHECK(parts[3] == "string");
        }

        {
            String s = " i am a string thing_thingy";
            auto parts = s.split(" ");
            CHECK(parts.length == 6);

            CHECK(parts[0] == "");
            CHECK(parts[1] == "i");
            CHECK(parts[2] == "am");
            CHECK(parts[3] == "a");
            CHECK(parts[4] == "string");
            CHECK(parts[5] == "thing_thingy");
        }
    }

    SECTION("trim_left") {
        auto trimmed = [](String s, String whitespace = " ") {
            s.trim_left(whitespace);
            return s;
        };
        CHECK(trimmed("") == String(""));
        CHECK(trimmed(" ") == String(""));
        CHECK(trimmed("  x") == String("x"));
        CHECK(trimmed("   a string") == String("a string"));
        CHECK(trimmed(" a  a string") == String("a  a string"));
        CHECK(trimmed(" a a string", " a") == String("string"));
        CHECK(trimmed(" a a string", " as") == String("tring"));
    }

    SECTION("trim_right") {
        auto trimmed = [](String s, String whitespace = " ") {
            s.trim_right(whitespace);
            return s;
        };

        CHECK(trimmed("") == String(""));
        CHECK(trimmed("     ") == String(""));
        CHECK(trimmed("_                      ") == String("_"));
        CHECK(trimmed("_                      _") == String("_                      _"));
        CHECK(trimmed("x  ") == String("x"));
        CHECK(trimmed("a string   ") == String("a string"));
        CHECK(trimmed("St. Stream 23 ") == String("St. Stream 23"));
        CHECK(trimmed("St. Stream 23 ", " 3") == String("St. Stream 2"));
    }

    SECTION("trim") {
        auto trimmed = [](String s, String whitespace = " ") {
            s.trim(whitespace);
            return s;
        };

        CHECK(trimmed("") == String(""));
        CHECK(trimmed("     ") == String(""));
        CHECK(trimmed("   _                      ") == String("_"));
        CHECK(trimmed("_                      _") == String("_                      _"));
        CHECK(trimmed("  x  ") == String("x"));
        CHECK(trimmed(" a string   ") == String("a string"));
        CHECK(trimmed("  St. Stream 23 ") == String("St. Stream 23"));
        CHECK(trimmed("333  3St. Stream 23 ", " 3") == String("St. Stream 2"));
    }
}
