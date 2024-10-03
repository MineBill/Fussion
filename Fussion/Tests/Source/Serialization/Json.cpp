#include <Fussion/Serialization/JsonSerializer.h>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Fussion;

TEST_CASE("JsonSerializer")
{
    JsonSerializer js;
    js.Initialize();

    SECTION("Simple")
    {
        js.Write("age", 21);

        auto const expected = "{\n"
                              R"(  "age": 21)"
                              "\n"
                              "}";
        CHECK(js.ToString() == expected);
    }

    SECTION("Numbers")
    {
        js.Write("small_number", 123);
        js.Write("big_number", 120931230123810);
        js.Write("float", 4.12);

        auto const expected = "{\n"
                              R"(  "small_number": 123,)"
                              "\n"
                              R"(  "big_number": 120931230123810,)"
                              "\n"
                              R"(  "float": 4.12)"
                              "\n"
                              "}";

        CHECK(js.ToString() == expected);
    }

    SECTION("Strings - const char*")
    {
        js.Write("name", "Mike");
        js.Write("street", "Someweirdstreetname 47");

        auto const expected = "{\n"
                              R"(  "name": "Mike",)"
                              "\n"
                              R"(  "street": "Someweirdstreetname 47")"
                              "\n"
                              "}";

        CHECK(js.ToString() == expected);
    }

    SECTION("Strings - std::string_view")
    {
        using namespace std::string_view_literals;
        js.Write("name", "Mike"sv);
        js.Write("street", "Someweirdstreetname 47"sv);

        auto const expected = "{\n"
                              R"(  "name": "Mike",)"
                              "\n"
                              R"(  "street": "Someweirdstreetname 47")"
                              "\n"
                              "}";

        CHECK(js.ToString() == expected);
    }

    SECTION("Strings - std::string")
    {
        using namespace std::string_literals;
        js.Write("name", "Mike"s);
        js.Write("street", "Someweirdstreetname 47"s);

        auto const expected = "{\n"
                              R"(  "name": "Mike",)"
                              "\n"
                              R"(  "street": "Someweirdstreetname 47")"
                              "\n"
                              "}";

        CHECK(js.ToString() == expected);
    }

    SECTION("Arrays")
    {
        using namespace std::string_view_literals;
        std::vector numbers = { 1, 2, 3 };

        js.WriteCollection<s32>("numbers"sv, numbers);

        SECTION("Single")
        {

            auto const expected = "{\n"
                                  R"(  "numbers": [)"
                                  "\n"
                                  R"(    1,)"
                                  "\n"
                                  R"(    2,)"
                                  "\n"
                                  R"(    3)"
                                  "\n"
                                  "  ]\n"
                                  "}";

            CHECK(js.ToString() == expected);
        }

        SECTION("Nested")
        {
            std::vector other_numbers = { 11, 22, 33 };
            js.WriteCollection("other_numbers"sv, other_numbers);

            auto const expected = "{\n"
                                  R"(  "numbers": [)"
                                  "\n"
                                  R"(    1,)"
                                  "\n"
                                  R"(    2,)"
                                  "\n"
                                  R"(    3)"
                                  "\n"
                                  "  ],\n"
                                  R"(  "other_numbers": [)"
                                  "\n"
                                  R"(    11,)"
                                  "\n"
                                  R"(    22,)"
                                  "\n"
                                  R"(    33)"
                                  "\n"
                                  "  ]\n"
                                  "}";

            CHECK(js.ToString() == expected);
        }
    }

    SECTION("Class")
    {
        SECTION("Single")
        {
            class MyClass : public ISerializable {
            public:
                std::string Name { "Pepegas" };
                u32 Age { 44 };

                virtual ~MyClass() override = default;

                virtual void Serialize(Serializer& serializer) const override
                {
                    serializer.Write("Name", Name);
                    serializer.Write("Age", Age);
                }
            };

            MyClass my_class;
            js.Write("my_class", my_class);

            auto const expected = "{\n"
                                  R"(  "my_class": {)"
                                  "\n"
                                  R"(    "Name": "Pepegas",)"
                                  "\n"
                                  R"(    "Age": 44)"
                                  "\n"
                                  "  }\n"
                                  "}";

            CHECK(js.ToString() == expected);
        }

        SECTION("Nested")
        {
            class Vector : public ISerializable {
            public:
                f32 X {}, Y {}, Z {};
                virtual ~Vector() override = default;

                virtual void Serialize(Serializer& serializer) const override
                {
                    serializer.Write("X", X);
                    serializer.Write("Y", Y);
                    serializer.Write("Z", Z);
                }
            };

            class MyClass : public ISerializable {
            public:
                std::string Name { "Pepegas" };
                u32 Age { 44 };
                Vector Position;

                virtual ~MyClass() override = default;

                virtual void Serialize(Serializer& serializer) const override
                {
                    serializer.Write("Name", Name);
                    serializer.Write("Age", Age);
                    serializer.Write("Position", Position);
                }
            };

            MyClass my_class;
            js.Write("my_class", my_class);

            auto const expected = "{\n"
                                  R"(  "my_class": {)"
                                  "\n"
                                  R"(    "Name": "Pepegas",)"
                                  "\n"
                                  R"(    "Age": 44,)"
                                  "\n"
                                  R"(    "Position": {)"
                                  "\n"
                                  R"(      "X": 0.0,)"
                                  "\n"
                                  R"(      "Y": 0.0,)"
                                  "\n"
                                  R"(      "Z": 0.0)"
                                  "\n"
                                  "    }\n"
                                  "  }\n"
                                  "}";

            CHECK(js.ToString() == expected);
        }
    }

    SECTION("Mixed")
    {
        class Vector : public ISerializable {
        public:
            f32 X {}, Y {}, Z {};
            std::vector<bool> Truths { false, false, true, false, true };
            virtual ~Vector() override = default;

            virtual void Serialize(Serializer& serializer) const override
            {
                serializer.Write("X", X);
                serializer.Write("Y", Y);
                serializer.Write("Z", Z);
                serializer.WriteCollection("Truths", Truths);
            }
        };
        class MyClass : public ISerializable {
        public:
            std::string Name { "Pepegas" };
            std::vector<s32> CoolNumbers { 3, 1, 4 };
            u32 Age { 44 };
            Vector Position;

            virtual ~MyClass() override = default;

            virtual void Serialize(Serializer& serializer) const override
            {
                serializer.Write("Name", Name);
                serializer.WriteCollection("CoolNumbers", CoolNumbers);
                serializer.Write("Age", Age);
                serializer.Write("Position", Position);
            }
        };

        MyClass my_class;
        js.Write("my_class", my_class);

        auto const expected =
            R"json({
  "my_class": {
    "Name": "Pepegas",
    "CoolNumbers": [
      3,
      1,
      4
    ],
    "Age": 44,
    "Position": {
      "X": 0.0,
      "Y": 0.0,
      "Z": 0.0,
      "Truths": [
        false,
        false,
        true,
        false,
        true
      ]
    }
  }
})json";

        CHECK(js.ToString() == expected);
    }

    SECTION("Array of objects")
    {
        class Vector : public ISerializable {
        public:
            f32 X {}, Y {}, Z {};
            Vector(f32 x, f32 y, f32 z)
                : X(x)
                , Y(y)
                , Z(z)
            { }
            virtual ~Vector() override = default;

            virtual void Serialize(Serializer& ctx) const override
            {
                FSN_SERIALIZE_MEMBER(X);
                FSN_SERIALIZE_MEMBER(Y);
                FSN_SERIALIZE_MEMBER(Z);
            }
        };
        std::vector positions = {
            Vector { 1.5f, 2.5f, 3.5f },
            Vector { 1, 2, 3 }
        };

        js.WriteCollection("positions", positions);

        auto const expected =
            R"json({
  "positions": [
    {
      "X": 1.5,
      "Y": 2.5,
      "Z": 3.5
    },
    {
      "X": 1.0,
      "Y": 2.0,
      "Z": 3.0
    }
  ]
})json";

        CHECK(js.ToString() == expected);
    }

    SECTION("Array of arrays")
    {
        std::vector arrays = {
            std::vector { 1, 2, 3 },
            std::vector { 1, 2, 3 },
            std::vector { 56, 12, 45, 12 }
        };

        js.WriteCollection("arrays", arrays);

        auto const expected =
            R"json({
  "arrays": [
    [
      1,
      2,
      3
    ],
    [
      1,
      2,
      3
    ],
    [
      56,
      12,
      45,
      12
    ]
  ]
})json";

        CHECK(js.ToString() == expected);
    }
}

class Point final : public ISerializable {
public:
    f32 X {}, Y {};
    Point() = default;
    Point(f32 x, f32 y)
        : X(x)
        , Y(y)
    { }

    bool operator==(Point const& other) const
    {
        return Math::IsZero(Math::Abs(X - other.X)) && Math::IsZero(Math::Abs(Y - other.Y));
    }

    virtual void Deserialize(Deserializer& ctx) override
    {
        FSN_DESERIALIZE_MEMBER(X);
        FSN_DESERIALIZE_MEMBER(Y);
    }
};

std::ostream& operator<<(std::ostream& os, Point const& value)
{
    os << "Point(" << value.X << ", " << value.Y << ")";
    return os;
}

TEST_CASE("JsonDeserializer")
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    SECTION("Numbers")
    {
        auto const json = R"({
  "number": 3.14,
  "int": 44
})";
        JsonDeserializer ds(json);

        f32 number;
        s32 integer;
        ds.Read("number"sv, number);
        ds.Read("int"sv, integer);

        CHECK(number == 3.14f);
        CHECK(integer == 44);
    }

    SECTION("Strings")
    {
        auto const json = R"({
  "name": "Mike",
  "street": "Somewhere 44"
})";
        JsonDeserializer ds(json);

        std::string name, street;
        ds.Read("name"sv, name);
        ds.Read("street"sv, street);

        CHECK(name == "Mike"s);
        CHECK(street == "Somewhere 44"s);
    }

    SECTION("Arrays")
    {
        SECTION("Numbers")
        {
            auto const json = R"({
  "numbers": [
    44,
    22,
    123
  ]
})";
            JsonDeserializer ds(json);

            std::vector<s32> numbers {};
            ds.ReadCollection("numbers", numbers);

            CHECK(numbers == std::vector { 44, 22, 123 });
        }

        SECTION("Strings")
        {
            auto const json = R"({
  "names": [
    "Mike",
    "John",
    "Nick"
  ]
})";
            JsonDeserializer ds(json);

            std::vector<std::string> names {};
            ds.ReadCollection("names", names);

            CHECK(names == std::vector { "Mike"s, "John"s, "Nick"s });
        }

        SECTION("Mixed")
        {
            SECTION("Sequential")
            {
                auto const json = R"({
  "numbers": [
    44,
    22,
    123
  ],
  "names": [
    "Mike",
    "John",
    "Nick"
  ]
})";
                JsonDeserializer ds(json);

                std::vector<std::string> names {};
                ds.ReadCollection("names", names);

                CHECK(names == std::vector { "Mike"s, "John"s, "Nick"s });

                std::vector<s32> numbers {};
                ds.ReadCollection("numbers", numbers);

                CHECK(numbers == std::vector { 44, 22, 123 });
            }
        }

        SECTION("Array of arrays")
        {
            auto const json = R"({
  "numbers": [
    [
      1,
      2,
      3
    ],
    [
      111,
      222,
      333,
      222
    ]
  ]
})";

            JsonDeserializer ds(json);

            std::vector<std::vector<s32>> numbers {};
            ds.ReadCollection("numbers", numbers);

            REQUIRE(numbers.size() == 2);
            CHECK(numbers[0].size() == 3);
            CHECK(numbers[1].size() == 4);
        }

        SECTION("Array of objects")
        {
            auto const json = R"({
  "points": [
    {
      "X": 1,
      "Y": 3
    },
    {
      "X": 11,
      "Y": 33
    },
    {
      "X": 51,
      "Y": 9532
    }
  ]
})";

            JsonDeserializer ds(json);

            std::vector<Point> points {};
            ds.ReadCollection("points", points);

            CHECK(points == std::vector<Point> { { 1, 3 }, { 11, 33 }, { 51, 9532 } });
        }
    }

    SECTION("Objects")
    {
        auto const json = R"({
  "point": {
    "X": 11,
    "Y": 22
  },
  "extra": {
    "aaa": 2
  }
})";
        JsonDeserializer ds(json);
        Point p;
        ds.Read("point", p);

        CHECK(p == Point { 11.f, 22.f });
    }

    SECTION("Missing")
    {
        SECTION("Numbers")
        {
            auto const json = R"({
  "numbers": 3.14,
  "ints": 44
})";
            JsonDeserializer ds(json);

            f32 number = 12.f;
            s32 integer = 255;
            ds.Read("number"sv, number);
            ds.Read("int"sv, integer);

            CHECK(number == 12.f);
            CHECK(integer == 255);
        }

        SECTION("Array")
        {
            auto const json = "{}";

            JsonDeserializer ds(json);

            std::vector<s32> numbers {};
            ds.ReadCollection("numbers", numbers);

            CHECK(numbers.empty());
        }

        SECTION("Object")
        {
            auto const json = "{}";

            JsonDeserializer ds(json);

            Point p;
            ds.Read("p", p);

            CHECK(p == Point());
        }

        SECTION("Partial Object")
        {
            auto const json = R"({
  "p": {
    "Y": 2
  }
})";

            JsonDeserializer ds(json);

            Point p(100, 100);
            ds.Read("p", p);

            CHECK(p == Point(100, 2));
        }
    }
}
