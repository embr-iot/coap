#include <embr/coap/options/decode.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("options decoding", "[decode][options]")
{
    SECTION("numbers")
    {
        constexpr uint8_t val1[] { 0x1E, 0x12, 0x34 };
        coap::options::numbers number;
        unsigned length;

        const uint8_t* out1 = coap::options::delta_length_decode(val1, 0, &number, &length);

        REQUIRE(out1 == val1 + 3);
        REQUIRE(number == 1);
        REQUIRE(length == 0x1234 + 269);
    }
}
