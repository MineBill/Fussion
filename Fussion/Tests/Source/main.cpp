#include "../../Source/Fussion/Core/SmallVector.h"
#include "Fussion/Core/Clap.h"

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

    SECTION("Paths")
    {
        SECTION("Windows Paths")
        {
            clap.Reset("-Project=W:/some/random/path");
            clap.Parse();

            auto project = clap.Get<std::string>("Project");
            REQUIRE(project.has_value());
            REQUIRE(*project == "W:/some/random/path");
        }

        SECTION("Unix Paths")
        {
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

int main(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
