#include <catch2/catch_test_macros.hpp>
#include "ringbuffer.h"

TEST_CASE("RingbufferView")
{
    GenericBuffer storage;
    REQUIRE(storage.Init(4));

    // test write head to the right of the read head
    RingbufferView ringbuffer(storage);
    REQUIRE(ringbuffer.GetReadableCount() == 0);
    REQUIRE(ringbuffer.GetWritableCount() == 4);
    ringbuffer.UncheckedWriteOne<uint8_t>(1);
    REQUIRE(ringbuffer.GetReadableCount() == 1);
    REQUIRE(ringbuffer.GetWritableCount() == 3);
    ringbuffer.UncheckedWriteOne<uint8_t>(2);
    ringbuffer.UncheckedWriteOne<uint8_t>(3);
    REQUIRE(ringbuffer.GetReadableCount() == 3);
    REQUIRE(ringbuffer.GetWritableCount() == 1);

    uint8_t x = 0;
    ringbuffer.UncheckedReadOne<uint8_t>(x);
    REQUIRE(x == 1);
    REQUIRE(ringbuffer.GetReadableCount() == 2);
    REQUIRE(ringbuffer.GetWritableCount() == 2);
    ringbuffer.UncheckedReadOne<uint8_t>(x);
    REQUIRE(x == 2);
    REQUIRE(ringbuffer.GetReadableCount() == 1);
    REQUIRE(ringbuffer.GetWritableCount() == 3);
    ringbuffer.UncheckedReadOne<uint8_t>(x);
    REQUIRE(x == 3);
    REQUIRE(ringbuffer.GetReadableCount() == 0);
    REQUIRE(ringbuffer.GetWritableCount() == 4);

    // test write head to the left of the read head
    ringbuffer.UncheckedWriteOne<uint8_t>(1); // w/r index 0
    ringbuffer.UncheckedReadOne<uint8_t>(x);
    ringbuffer.UncheckedWriteOne<uint8_t>(2); // w/r index 1
    ringbuffer.UncheckedReadOne<uint8_t>(x);
    ringbuffer.UncheckedWriteOne<uint8_t>(3); // w/r index 2
    ringbuffer.UncheckedReadOne<uint8_t>(x);
    ringbuffer.UncheckedWriteOne<uint8_t>(4); // w index 3
    ringbuffer.UncheckedWriteOne<uint8_t>(5); // w index 0
    REQUIRE(ringbuffer.GetReadableCount() == 2);
    REQUIRE(ringbuffer.GetWritableCount() == 2);
    ringbuffer.UncheckedReadOne<uint8_t>(x); // r index 3 with w at 1
    REQUIRE(x == 4);
    ringbuffer.UncheckedReadOne<uint8_t>(x); // r index 0 with w at 1
    REQUIRE(x == 5);

    storage.Free();
}
