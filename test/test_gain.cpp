#include "common/gain.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Gain parsing")
{
    using namespace common;

    float gain;

    // Invalid parses
    REQUIRE(ParseGain("db", gain) != ParseGainResult{});
    REQUIRE(ParseGain("-db", gain) != ParseGainResult{});
    REQUIRE(ParseGain("+db", gain) != ParseGainResult{});
    REQUIRE(ParseGain("+", gain) != ParseGainResult{});
    REQUIRE(ParseGain("-", gain) != ParseGainResult{});
    REQUIRE(ParseGain("", gain) != ParseGainResult{});
    REQUIRE(ParseGain(".", gain) != ParseGainResult{});
    REQUIRE(ParseGain("1..", gain) != ParseGainResult{});
    REQUIRE(ParseGain("0x2", gain) != ParseGainResult{});

    // Looks valid, but would produce a value out of range
    REQUIRE(ParseGain("-0.5", gain) != ParseGainResult{});
    REQUIRE(ParseGain("999999999999999999999999999999999999999999999999999999999", gain) != ParseGainResult{});

    // Valid parses
    REQUIRE(ParseGain("0.5", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(0.5, 0.01));

    REQUIRE(ParseGain(".5", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(0.5, 0.01));

    REQUIRE(ParseGain("2.5", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(2.5, 0.01));

    REQUIRE(ParseGain("6db", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(2.0, 0.01));

    REQUIRE(ParseGain("+6db", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(2.0, 0.01));

    REQUIRE(ParseGain("-6db", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(0.5, 0.01));

    REQUIRE(ParseGain("+12db", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(4.0, 0.10));

    REQUIRE(ParseGain("-12db", gain) == ParseGainResult{});
    REQUIRE_THAT(gain, Catch::Matchers::WithinAbs(0.25, 0.01));
}
