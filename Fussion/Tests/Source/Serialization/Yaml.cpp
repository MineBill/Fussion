#include <Fussion/Serialization/YamlSerializer.h>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Fussion;

TEST_CASE("YamlSerializer Initialization", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();
    REQUIRE(serializer.ToString() == "{}"); // Assuming ToString() should return an empty string after initialization
}

TEST_CASE("YamlSerializer Write integer values", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    SECTION("Write s8 value")
    {
        serializer.Write("int8", static_cast<s8>(-128));
        REQUIRE(serializer.ToString() == "int8: -128");
    }

    SECTION("Write u8 value")
    {
        serializer.Write("uint8", static_cast<u8>('g'));
        REQUIRE(serializer.ToString() == R"(uint8: g)");
    }

    SECTION("Write u8 symbol")
    {
        serializer.Write("uint8", static_cast<u8>('/'));
        REQUIRE(serializer.ToString() == R"(uint8: "/")");
    }

    SECTION("Write s16 value")
    {
        serializer.Write("int16", static_cast<s16>(-32768));
        REQUIRE(serializer.ToString() == "int16: -32768");
    }

    SECTION("Write u16 value")
    {
        serializer.Write("uint16", static_cast<u16>(65535));
        REQUIRE(serializer.ToString() == "uint16: 65535");
    }

    SECTION("Write s32 value")
    {
        serializer.Write("int32", static_cast<s32>(-2147483648));
        REQUIRE(serializer.ToString() == "int32: -2147483648");
    }

    SECTION("Write u32 value")
    {
        serializer.Write("uint32", static_cast<u32>(4294967295));
        REQUIRE(serializer.ToString() == "uint32: 4294967295");
    }

    SECTION("Write s64 value")
    {
        serializer.Write("int64", static_cast<s64>(-9223372036854775807));
        REQUIRE(serializer.ToString() == "int64: -9223372036854775807");
    }

    SECTION("Write u64 value")
    {
        serializer.Write("uint64", static_cast<u64>(18446744073709551615U));
        REQUIRE(serializer.ToString() == "uint64: 18446744073709551615");
    }
}

TEST_CASE("YamlSerializer Write floating point values", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    SKIP("yaml-cpp pr needs to be merged for proper fp formatting");
    SECTION("Write f32 value")
    {
        serializer.Write("float32", static_cast<f32>(3.14f));
        REQUIRE(serializer.ToString() == "float32: 3.14");
    }

    SECTION("Write f64 value")
    {
        serializer.Write("float64", static_cast<f64>(2.718281828459045));
        REQUIRE(serializer.ToString() == "float64: 2.718281828459045");
    }
}

TEST_CASE("YamlSerializer Write boolean and string values", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    SECTION("Write boolean value")
    {
        serializer.Write("bool", true);
        REQUIRE(serializer.ToString() == "bool: true");
    }

    SECTION("Write string_view value")
    {
        serializer.Write("string_view", std::string_view("test string_view"));
        REQUIRE(serializer.ToString() == "string_view: test string_view");
    }

    SECTION("Write const char* value")
    {
        serializer.Write("const_char_ptr", "test const char*");
        REQUIRE(serializer.ToString() == "const_char_ptr: test const char*");
    }
}

class MockSerializable : public ISerializable {
public:
    void Serialize(Serializer& serializer) const override
    {
        serializer.Write("mock", "serialized");
    }
};

TEST_CASE("YamlSerializer Write ISerializable object", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    MockSerializable mock;
    serializer.Write("object", mock);
    REQUIRE(serializer.ToString() == "object:\n  mock: serialized");
}

TEST_CASE("YamlSerializer Begin and End Object", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    serializer.BeginObject("object", 1);
    serializer.Write("key", "value");
    serializer.EndObject();
    REQUIRE(serializer.ToString() == "object:\n  key: value");
}

TEST_CASE("YamlSerializer Begin and End Array", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    serializer.BeginArray("array", 3);
    serializer.Write("0", 1);
    serializer.Write("1", 2);
    serializer.Write("2", 3);
    serializer.EndArray();
    REQUIRE(serializer.ToString() == "array:\n  - 1\n  - 2\n  - 3");
}

TEST_CASE("YamlSerializer Nested Objects and Arrays", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    SECTION("Nested Object")
    {
        serializer.BeginObject("root", 1);
        serializer.BeginObject("child", 1);
        serializer.Write("key", "value");
        serializer.EndObject();
        serializer.EndObject();
        REQUIRE(serializer.ToString() == R"(root:
  child:
    key: value)");
    }

    SECTION("Array of Objects")
    {
        serializer.BeginArray("array_of_objects", 2);
        serializer.BeginObject("", 1);
        serializer.Write("key1", "value1");
        serializer.EndObject();
        serializer.BeginObject("", 1);
        serializer.Write("key2", "value2");
        serializer.EndObject();
        serializer.EndArray();
        REQUIRE(serializer.ToString() == R"(array_of_objects:
  - key1: value1
  - key2: value2)");
    }

    SECTION("Object with Array")
    {
        serializer.BeginObject("object_with_array", 1);
        serializer.BeginArray("array", 2);
        serializer.Write("", "value1");
        serializer.Write("", "value2");
        serializer.EndArray();
        serializer.EndObject();
        REQUIRE(serializer.ToString() == R"(object_with_array:
  array:
    - value1
    - value2)");
    }
}

TEST_CASE("YamlSerializer Complex Serialization", "[YamlSerializer]")
{
    YamlSerializer serializer;
    serializer.Initialize();

    SECTION("Complex Nested Structure")
    {
        serializer.BeginObject("root", 3);

        serializer.Write("simple_key", "simple_value");

        serializer.BeginObject("nested_object", 1);
        serializer.Write("nested_key", "nested_value");
        serializer.EndObject();

        serializer.BeginArray("nested_array", 3);
        serializer.Write("", "array_value1");
        serializer.Write("", "array_value2");
        serializer.BeginObject("", 1);
        serializer.Write("array_object_key", "array_object_value");
        serializer.EndObject();
        serializer.EndArray();

        serializer.EndObject();

        REQUIRE(serializer.ToString() == R"(root:
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
        serializer.BeginObject("level1", 1);
        serializer.BeginObject("level2", 1);
        serializer.BeginObject("level3", 1);
        serializer.Write("key", "deep_value");
        serializer.EndObject();
        serializer.EndObject();
        serializer.EndObject();
        REQUIRE(serializer.ToString() == R"(level1:
  level2:
    level3:
      key: deep_value)");
    }
}
