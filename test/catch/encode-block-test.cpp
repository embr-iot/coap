#include "test-data.h"

#include <embr/coap/options/block/value.h>

#include <catch2/catch_all.hpp>

using namespace embr::coap;

constexpr uint8_t expected_data1[] = { O_BLOCK_DATA1 };

TEST_CASE("block encoding", "[encode][block]")
{
    using options::block_value;

    uint8_t data[4];

    block_value bv{};

    REQUIRE(bv.size() == 16);

    // asserts, as it should
    //bv = block_value(0x1000000, false, 0);
    //constexpr block_value bv_bad{0x1000000, false, 0};

    bool r = bv.decode(expected_data1, sizeof(expected_data1));
    REQUIRE(r);

    REQUIRE(bv.num() == 0x1234);
    REQUIRE(bv.more() == false);
    REQUIRE(bv.size() == 32);

    int r2 = bv.encode(data);

    REQUIRE(r2 == 3);

    REQUIRE_THAT(estd::span(data, 3), Catch::Matchers::RangeEquals(estd::span(expected_data1)));

    bv.reset();
    r2 = bv.encode(data);

    REQUIRE(r2 == 0);
}