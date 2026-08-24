#include "test-data.h"

#include <embr/coap/options/block/value.h>

#include <catch2/catch_all.hpp>

using namespace embr::coap;

constexpr uint8_t expected_data1[] = { O_BLOCK_DATA1 };

TEST_CASE("block encoding", "[encode][block]")
{
    using options::block_value;
    using options::block_trailing_byte;

    block_trailing_byte btb1(3, false, 1);

    REQUIRE(btb1.num() == 3);
    REQUIRE(btb1.szx() == 1);

    uint8_t data[4];

    constexpr block_value bv{0x1234, false, 1};

    int r2 = bv.encode(data);

    REQUIRE(r2 == 3);

    REQUIRE_THAT(estd::span(data, 3), Catch::Matchers::RangeEquals(estd::span(expected_data1)));

    constexpr block_value bv1{};

    r2 = bv1.encode(data);

    REQUIRE(r2 == 0);
}