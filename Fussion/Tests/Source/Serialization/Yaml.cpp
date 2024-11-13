#include <Fussion/Serialization/YamlSerializer.h>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Fussion;

TEST_CASE("YamlSerializer Initialization", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();
    REQUIRE(serializer.to_string() == "{}"); // Assuming ToString() should return an empty string after initialization
}

TEST_CASE("YamlSerializer Write integer values", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    SECTION("Write s8 value")
    {
        serializer.write("int8", static_cast<s8>(-128));
        REQUIRE(serializer.to_string() == "int8: -128");
    }

    SECTION("Write u8 value")
    {
        serializer.write("uint8", static_cast<u8>('g'));
        REQUIRE(serializer.to_string() == R"(uint8: g)");
    }

    SECTION("Write u8 symbol")
    {
        serializer.write("uint8", static_cast<u8>('/'));
        REQUIRE(serializer.to_string() == R"(uint8: "/")");
    }

    SECTION("Write s16 value")
    {
        serializer.write("int16", static_cast<s16>(-32768));
        REQUIRE(serializer.to_string() == "int16: -32768");
    }

    SECTION("Write u16 value")
    {
        serializer.write("uint16", static_cast<u16>(65535));
        REQUIRE(serializer.to_string() == "uint16: 65535");
    }

    SECTION("Write s32 value")
    {
        serializer.write("int32", static_cast<s32>(-2147483648));
        REQUIRE(serializer.to_string() == "int32: -2147483648");
    }

    SECTION("Write u32 value")
    {
        serializer.write("uint32", static_cast<u32>(4294967295));
        REQUIRE(serializer.to_string() == "uint32: 4294967295");
    }

    SECTION("Write s64 value")
    {
        serializer.write("int64", static_cast<s64>(-9223372036854775807));
        REQUIRE(serializer.to_string() == "int64: -9223372036854775807");
    }

    SECTION("Write u64 value")
    {
        serializer.write("uint64", static_cast<u64>(18446744073709551615U));
        REQUIRE(serializer.to_string() == "uint64: 18446744073709551615");
    }
}

TEST_CASE("YamlSerializer Write floating point values", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    SKIP("yaml-cpp pr needs to be merged for proper fp formatting");
    SECTION("Write f32 value")
    {
        serializer.write("float32", static_cast<f32>(3.14f));
        REQUIRE(serializer.to_string() == "float32: 3.14");
    }

    SECTION("Write f64 value")
    {
        serializer.write("float64", static_cast<f64>(2.718281828459045));
        REQUIRE(serializer.to_string() == "float64: 2.718281828459045");
    }
}

TEST_CASE("YamlSerializer Write boolean and string values", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    SECTION("Write boolean value")
    {
        serializer.write("bool", true);
        REQUIRE(serializer.to_string() == "bool: true");
    }

    SECTION("Write string_view value")
    {
        serializer.write("string_view", std::string_view("test string_view"));
        REQUIRE(serializer.to_string() == "string_view: test string_view");
    }

    SECTION("Write const char* value")
    {
        serializer.write("const_char_ptr", "test const char*");
        REQUIRE(serializer.to_string() == "const_char_ptr: test const char*");
    }
}

class MockSerializable : public ISerializable {
public:
    void serialize(Serializer& serializer) const override
    {
        serializer.write("mock", "serialized");
    }
};

TEST_CASE("YamlSerializer Write ISerializable object", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    MockSerializable mock;
    serializer.write("object", mock);
    REQUIRE(serializer.to_string() == "object:\n  mock: serialized");
}

TEST_CASE("YamlSerializer Begin and End Object", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    serializer.begin_object("object", 1);
    serializer.write("key", "value");
    serializer.end_object();
    REQUIRE(serializer.to_string() == "object:\n  key: value");
}

TEST_CASE("YamlSerializer Begin and End Array", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    serializer.begin_array("array", 3);
    serializer.write("0", 1);
    serializer.write("1", 2);
    serializer.write("2", 3);
    serializer.end_array();
    REQUIRE(serializer.to_string() == "array:\n  - 1\n  - 2\n  - 3");
}

TEST_CASE("YamlSerializer Nested Objects and Arrays", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    SECTION("Nested Object")
    {
        serializer.begin_object("root", 1);
        serializer.begin_object("child", 1);
        serializer.write("key", "value");
        serializer.end_object();
        serializer.end_object();
        REQUIRE(serializer.to_string() == R"(root:
  child:
    key: value)");
    }

    SECTION("Array of Objects")
    {
        serializer.begin_array("array_of_objects", 2);
        serializer.begin_object("", 1);
        serializer.write("key1", "value1");
        serializer.end_object();
        serializer.begin_object("", 1);
        serializer.write("key2", "value2");
        serializer.end_object();
        serializer.end_array();
        REQUIRE(serializer.to_string() == R"(array_of_objects:
  - key1: value1
  - key2: value2)");
    }

    SECTION("Object with Array")
    {
        serializer.begin_object("object_with_array", 1);
        serializer.begin_array("array", 2);
        serializer.write("", "value1");
        serializer.write("", "value2");
        serializer.end_array();
        serializer.end_object();
        REQUIRE(serializer.to_string() == R"(object_with_array:
  array:
    - value1
    - value2)");
    }
}

TEST_CASE("YamlSerializer Complex Serialization", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.initialize();

    SECTION("Complex Nested Structure")
    {
        serializer.begin_object("root", 3);

        serializer.write("simple_key", "simple_value");

        serializer.begin_object("nested_object", 1);
        serializer.write("nested_key", "nested_value");
        serializer.end_object();

        serializer.begin_array("nested_array", 3);
        serializer.write("", "array_value1");
        serializer.write("", "array_value2");
        serializer.begin_object("", 1);
        serializer.write("array_object_key", "array_object_value");
        serializer.end_object();
        serializer.end_array();

        serializer.end_object();

        REQUIRE(serializer.to_string() == R"(root:
  simple_key: simple_value
  nested_object:
    nested_key: nested_value
  nested_array:
    - array_value1
    - array_value2
    - array_object_key: array_object_value)");
    }

    SECTION("Deeply Nested Structure")
    {
        serializer.begin_object("level1", 1);
        serializer.begin_object("level2", 1);
        serializer.begin_object("level3", 1);
        serializer.write("key", "deep_value");
        serializer.end_object();
        serializer.end_object();
        serializer.end_object();
        REQUIRE(serializer.to_string() == R"(level1:
  level2:
    level3:
      key: deep_value)");
    }
}
