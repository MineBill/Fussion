#include <Fussion/Math/Math.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
using namespace Fussion;

TEST_CASE("Math")
{
    SECTION("Floor") {
        SECTION("f32") {
            CHECK(Math::Floor(1.4f) == 1.0f);
            CHECK(Math::Floor(3.14f) == 3.0f);
            CHECK(Math::Floor(1.0f) == 1.0f);
            CHECK(Math::Floor(1234.123f) == 1234.0f);

            CHECK_FALSE(Math::Floor(1224.123f) == 1234.0f);
        }
        SECTION("f64") {
            CHECK(Math::Floor(1.4) == 1.0);
            CHECK(Math::Floor(1.0) == 1.0);
            CHECK(Math::Floor(3.14) == 3.0);
        }
    }

    SECTION("IsZero") {
        CHECK(Math::IsZero(0.0f));
        CHECK(Math::IsZero(0.0));
        CHECK_FALSE(Math::IsZero(1.0f));
        CHECK_FALSE(Math::IsZero(3.14));
    }

    SECTION("FloorLog2") {
        CHECK(Math::FloorLog2(1) == 0);
        CHECK(Math::FloorLog2(2) == 1);
        CHECK(Math::FloorLog2(3) == 1);
        CHECK(Math::FloorLog2(7) == 2);
        CHECK(Math::FloorLog2(8) == 3);
        CHECK(Math::FloorLog2(15) == 3);
        CHECK(Math::FloorLog2(16) == 4);
        CHECK(Math::FloorLog2(31) == 4);
        CHECK(Math::FloorLog2(32) == 5);
        CHECK(Math::FloorLog2(63) == 5);
        CHECK(Math::FloorLog2(64) == 6);
        CHECK(Math::FloorLog2(127) == 6);
        CHECK(Math::FloorLog2(128) == 7);
        CHECK(Math::FloorLog2(255) == 7);
        CHECK(Math::FloorLog2(256) == 8);
        CHECK(Math::FloorLog2(511) == 8);
        CHECK(Math::FloorLog2(512) == 9);
        CHECK(Math::FloorLog2(1023) == 9);
        CHECK(Math::FloorLog2(1024) == 10);
    }
}
