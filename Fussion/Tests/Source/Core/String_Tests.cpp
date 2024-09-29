#include <iostream>
#include <Fussion/Core/String.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
using namespace Fussion;

namespace Catch {
    template<>
    struct StringMaker<String> {
        static std::string convert(String const& value)
        {
            return std::string(value.data.ptr, value.len());
        }
    };
}

// TEST_CASE("String")
// {
//     SECTION("String literals") {
//         String s = "Hello World";
//         CHECK(s.len() == sizeof("Hello World") - 1);
//     }
//
//     SECTION("Alloc") {
//         String s = String::alloc("AWDWAD");
//         CHECK(s.len() == sizeof("AWDWAD") - 1);
//         CHECK(s[0] == 'A');
//
//         s[0] = 1;
//
//         CHECK(s[0] == 1);
//     }
//
//     SECTION("Clone") {
//         String s = String::alloc("Jonathan");
//         String view = s.clone();
//         s.free();
//     }
//
//     SECTION("index of") {
//         String s = "This is a string";
//         auto space = s.index_of(" ");
//         CHECK(space.has_value());
//         CHECK(space.value() == 4);
//
//         CHECK(s.index_of("_").is_empty());
//
//         auto is = s.index_of("is");
//         CHECK(is.value() == 2);
//     }
//
//     SECTION("split") {
//         {
//             String s = "this_is_a_string";
//             auto parts = s.split("_");
//             CHECK(parts.length == 4);
//
//             CHECK(parts[0] == "this");
//             CHECK(parts[1] == "is");
//             CHECK(parts[2] == "a");
//             CHECK(parts[3] == "string");
//         }
//
//         {
//             String s = " i am a string thing_thingy";
//             auto parts = s.split(" ");
//             CHECK(parts.length == 6);
//
//             CHECK(parts[0] == "");
//             CHECK(parts[1] == "i");
//             CHECK(parts[2] == "am");
//             CHECK(parts[3] == "a");
//             CHECK(parts[4] == "string");
//             CHECK(parts[5] == "thing_thingy");
//         }
//     }
//
//     SECTION("trim_left") {
//         auto trimmed = [](String s, String whitespace = " ") {
//             s.trim_left(whitespace);
//             return s;
//         };
//         CHECK(trimmed("") == String(""));
//         CHECK(trimmed(" ") == String(""));
//         CHECK(trimmed("  x") == String("x"));
//         CHECK(trimmed("   a string") == String("a string"));
//         CHECK(trimmed(" a  a string") == String("a  a string"));
//         CHECK(trimmed(" a a string", " a") == String("string"));
//         CHECK(trimmed(" a a string", " as") == String("tring"));
//     }
//
//     SECTION("trim_right") {
//         auto trimmed = [](String s, String whitespace = " ") {
//             s.trim_right(whitespace);
//             return s;
//         };
//
//         CHECK(trimmed("") == String(""));
//         CHECK(trimmed("     ") == String(""));
//         CHECK(trimmed("_                      ") == String("_"));
//         CHECK(trimmed("_                      _") == String("_                      _"));
//         CHECK(trimmed("x  ") == String("x"));
//         CHECK(trimmed("a string   ") == String("a string"));
//         CHECK(trimmed("St. Stream 23 ") == String("St. Stream 23"));
//         CHECK(trimmed("St. Stream 23 ", " 3") == String("St. Stream 2"));
//     }
//
//     SECTION("trim") {
//         auto trimmed = [](String s, String whitespace = " ") {
//             s.trim(whitespace);
//             return s;
//         };
//
//         CHECK(trimmed("") == String(""));
//         CHECK(trimmed("     ") == String(""));
//         CHECK(trimmed("   _                      ") == String("_"));
//         CHECK(trimmed("_                      _") == String("_                      _"));
//         CHECK(trimmed("  x  ") == String("x"));
//         CHECK(trimmed(" a string   ") == String("a string"));
//         CHECK(trimmed("  St. Stream 23 ") == String("St. Stream 23"));
//         CHECK(trimmed("333  3St. Stream 23 ", " 3") == String("St. Stream 2"));
//     }
// }

TEST_CASE("String constructor and basic functionality", "[String]")
{
    String empty_str;
    REQUIRE(empty_str.len() == 0);

    String hello_str("Hello");
    REQUIRE(hello_str.len() == 5);
    REQUIRE(hello_str[0] == 'H');
    REQUIRE(hello_str[4] == 'o');
    REQUIRE(hello_str == String("Hello"));
}

TEST_CASE("String clone and free", "[String]")
{
    String hello_str("Hello");
    auto cloned_str = hello_str.clone(mem::heap_allocator());

    REQUIRE(cloned_str == hello_str);
    REQUIRE(cloned_str.len() == hello_str.len());

    // Freeing resources for cloned string
    cloned_str.free(mem::heap_allocator());
    REQUIRE(cloned_str.len() == 0);
}

TEST_CASE("String split", "[String]")
{
    String csv_str("A,B,C");
    auto parts = csv_str.split(String(","), mem::heap_allocator());

    REQUIRE(parts.len() == 3);
    REQUIRE(parts[0] == String("A"));
    REQUIRE(parts[1] == String("B"));
    REQUIRE(parts[2] == String("C"));
}

TEST_CASE("String index_of", "[String]")
{
    String test_str("Find the needle in the haystack");

    auto maybe_index = test_str.index_of(String("needle"));
    REQUIRE(maybe_index.has_value());
    REQUIRE(maybe_index.unwrap() == 9);

    auto not_found = test_str.index_of(String("missing"));
    REQUIRE(!not_found.has_value());
}

TEST_CASE("String replace", "[String]") {
    // Normal case
    String test_str("foo bar baz");
    auto maybe_replaced_str = test_str.replace(String("bar"), String("qux"));
    REQUIRE(maybe_replaced_str.has_value());
    REQUIRE(maybe_replaced_str.unwrap() == String("foo qux baz"));
    REQUIRE(test_str == String("foo bar baz"));  // Original string should remain unchanged

    // Edge case: no replacement found
    maybe_replaced_str = test_str.replace(String("xyz"), String("abc"));
    REQUIRE(!maybe_replaced_str.has_value());  // No replacement, should return empty Maybe

    // Edge case: replacing with an empty string
    maybe_replaced_str = test_str.replace(String("bar"), String(""));
    REQUIRE(maybe_replaced_str.has_value());
    REQUIRE(maybe_replaced_str.unwrap() == String("foo  baz"));

    // Edge case: replacing in an empty string
    String empty_str;
    maybe_replaced_str = empty_str.replace(String("bar"), String("foo"));
    REQUIRE(!maybe_replaced_str.has_value());  // Empty string, should return empty Maybe

    // Edge case: replacing entire string
    maybe_replaced_str = test_str.replace(String("foo bar baz"), String("new"));
    REQUIRE(maybe_replaced_str.has_value());
    REQUIRE(maybe_replaced_str.unwrap() == String("new"));

    // Edge case: empty old_str
    maybe_replaced_str = test_str.replace(String(""), String("new"));
    REQUIRE(!maybe_replaced_str.has_value());  // Empty old_str, no replacement should occur
}

TEST_CASE("String trim", "[String]")
{
    String padded_str("   hello   ");

    padded_str.trim();
    REQUIRE(padded_str == String("hello"));

    String left_padded_str("   hello");
    left_padded_str.trim_left();
    REQUIRE(left_padded_str == String("hello"));

    String right_padded_str("hello   ");
    right_padded_str.trim_right();
    REQUIRE(right_padded_str == String("hello"));
}

TEST_CASE("String view", "[String]")
{
    String test_str("substring");
    String sub_view = test_str.view(3, 6); // should return "str"

    REQUIRE(sub_view == String("str"));
}

// TEST_CASE("String multi-dimensional subscript", "[String]") {
// #if HAS_MICROSHIT_FINALLY_IMPLEMENTED_MULTIDIMENSIONAL_SUBSCRIPT
//     String test_str("abcdef");
//     REQUIRE(test_str[0, 3] == String("abc"));
// #endif
// }

TEST_CASE("String alloc", "[String]")
{
    String allocated_str = String::alloc("heap allocated string", mem::heap_allocator());

    REQUIRE(allocated_str == String("heap allocated string"));
    allocated_str.free(mem::heap_allocator());
}

TEST_CASE("String equality operator", "[String]")
{
    String str1("Test");
    String str2("Test");
    String str3("Different");

    REQUIRE(str1 == str2);
    REQUIRE(!(str1 == str3));
}

TEST_CASE("String subscript operator", "[String]")
{
    String test_str("Hello");
    REQUIRE(test_str[0] == 'H');
    REQUIRE(test_str[4] == 'o');
}
