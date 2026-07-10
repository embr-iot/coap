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

        int counter = 0;

        estd::detail::basic_ispanbuf<const uint8_t> in(test::data1);
        decoder_type decoder(test::data1);

        // compiles, but doesn't seem to call f()
        //decode_exp(in, [&](const auto& o)
        //estd::errc err = decode(estd::span(test::data1), [&](const auto& o)
        estd::errc err = decoder.decode([&](const auto& o)
        {
            if constexpr(o.number == numbers::UriHost)
            {
                ++counter;
                REQUIRE(o.string() == "host");
            }
            else if constexpr(o.number == numbers::UriPath)
            {
                if(++counter == 2)
                    REQUIRE(o.string() == "v1");
                else
                    REQUIRE(o.string() == "t");
            }
            else
            {
                // NOTE: Just a sanity check that this compiles.  Otherwise not needed
                option2 o2 = o;
            }
        });

        REQUIRE(err == estd::errc{});
        REQUIRE(counter == 3);
    }
    SECTION("stateful decoder")
    {
        using namespace coap::options;

        estd::detail::basic_ispanbuf<const uint8_t> in(test::data1);
        stateful_decoder decoder;

        estd::errc err = decoder.decode(in, [](option2 o)
            {

            });
    }
}
