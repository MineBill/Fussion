#include <Fussion/Math/Math.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
using namespace Fussion;

TEST_CASE("Math")
{
    SECTION("Floor") {
        SECTION("f32") {
            CHECK(Math::floor(1.4f) == 1.0f);
            CHECK(Math::floor(3.14f) == 3.0f);
            CHECK(Math::floor(1.0f) == 1.0f);
            CHECK(Math::floor(1234.123f) == 1234.0f);

            CHECK_FALSE(Math::floor(1224.123f) == 1234.0f);
        }
        SECTION("f64") {
            CHECK(Math::floor(1.4) == 1.0);
            CHECK(Math::floor(1.0) == 1.0);
            CHECK(Math::floor(3.14) == 3.0);
        }
    }

    SECTION("IsZero") {
        CHECK(Math::is_zero(0.0f));
        CHECK(Math::is_zero(0.0));
        CHECK_FALSE(Math::is_zero(1.0f));
        CHECK_FALSE(Math::is_zero(3.14));
    }

    SECTION("FloorLog2") {
        CHECK(Math::floor_log2(1) == 0);
        CHECK(Math::floor_log2(2) == 1);
        CHECK(Math::floor_log2(3) == 1);
        CHECK(Math::floor_log2(7) == 2);
        CHECK(Math::floor_log2(8) == 3);
        CHECK(Math::floor_log2(15) == 3);
        CHECK(Math::floor_log2(16) == 4);
        CHECK(Math::floor_log2(31) == 4);
        CHECK(Math::floor_log2(32) == 5);
        CHECK(Math::floor_log2(63) == 5);
        CHECK(Math::floor_log2(64) == 6);
        CHECK(Math::floor_log2(127) == 6);
        CHECK(Math::floor_log2(128) == 7);
        CHECK(Math::floor_log2(255) == 7);
        CHECK(Math::floor_log2(256) == 8);
        CHECK(Math::floor_log2(511) == 8);
        CHECK(Math::floor_log2(512) == 9);
        CHECK(Math::floor_log2(1023) == 9);
        CHECK(Math::floor_log2(1024) == 10);
    }
}
