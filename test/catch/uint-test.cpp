#include <embr/coap/uint.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("uint")
{
    SECTION("decode")
    {
        constexpr uint8_t in_16[]       { 0x10 };
        constexpr uint8_t in_0001[]     { 0,    1 };
        constexpr uint8_t in_1234[]     { 4, 0xD2 };
        constexpr uint8_t in_123456[]   { 1, 0xE2, 0x40 };

        auto v = coap::uint_decode<unsigned>(in_16, 1);

        REQUIRE(v == 16);

        v = coap::uint_decode<unsigned>(in_0001, 2);

        REQUIRE(v == 1);

        v = coap::uint_decode<unsigned>(in_1234, 2);

        REQUIRE(v == 1234);

        v = coap::uint_decode<unsigned>(in_123456, 3);

        REQUIRE(v == 123456);
    }
}
