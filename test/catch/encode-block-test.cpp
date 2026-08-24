#include <embr/coap/options/block/value.h>

#include <catch2/catch_all.hpp>

using namespace embr::coap;

TEST_CASE("block encoding", "[encode][block]")
{
    options::block_value bv{};

    REQUIRE(bv.size() == 16);
}