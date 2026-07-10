#include "test-data.h"

#include <embr/coap/options/decode.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("options decoding", "[decode][options]")
{
    SECTION("numbers")
    {
        constexpr uint8_t val1[] { 0x1E, 0x12, 0x34 };
        coap::options::numbers number;
        unsigned length{3};

        SECTION("direct")
        {
            const uint8_t* out1 = coap::options::delta_length_decode(val1, 0, &number, &length);

            REQUIRE(out1 == val1 + 3);
            REQUIRE(number == 1);
            REQUIRE(length == 0x1234 + 269);
        }
        SECTION("state machine")
        {
            coap::options::delta_length_decoder d;

            bool b = d.decode_byte(val1[0]);
            REQUIRE(b);
            b = d.decode_byte(val1[1]);
            REQUIRE(b);
            b = d.decode_byte(val1[2]);
            REQUIRE(!b);

            REQUIRE(d.delta() == 1);
            REQUIRE(d.length() == 0x1234 + 269);
        }
    }
    SECTION("decoder")
    {
        using namespace coap::options;

        using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

        decoder_type decoder(test::data1);

        decoder.decode([] {});
    }
}
