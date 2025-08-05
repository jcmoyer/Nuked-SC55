#include "backend/bounded_ordered_bitset.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("BoundedOrderedBitSet")
{
    BoundedOrderedBitSet<6, uint32_t> s;
    static_assert(std::is_same_v<decltype(s)::UnderlyingType, uint8_t>);
    static_assert(sizeof(s) == 1);

    s.Include(5);
    s.Include(0);
    s.Include(2);
    REQUIRE(s.Size() == 3);
    REQUIRE(s.Contains(2));
    REQUIRE(s.Contains(0));
    REQUIRE(s.Contains(5));

    auto it = s.begin();
    REQUIRE(*it == 0);
    ++it;
    REQUIRE(*it == 2);
    ++it;
    REQUIRE(*it == 5);
    ++it;
    REQUIRE(it == s.end());

    s.Exclude(2);
    REQUIRE(s.Size() == 2);

    it = s.begin();
    REQUIRE(*it == 0);
    ++it;
    REQUIRE(*it == 5);
    ++it;
    REQUIRE(it == s.end());
}
