#include "test-data.h"

#include <embr/coap/internal/constants.h>
#include <embr/coap/options/decode.h>
#include <embr/coap/options/numbers.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("options decoding", "[decode][options]")
{
    SECTION("numbers")
    {
        constexpr uint8_t val1[] { 0x1E, 0x12, 0x34 };
        coap::options::numbers number;
        namespace constants = coap::constants;
        unsigned length{3};

        SECTION("direct")
        {
            const uint8_t* out1 = coap::options::delta_length_decode(val1, 0, &number, &length);

            REQUIRE(out1 == val1 + 3);
            REQUIRE(number == 1);
            REQUIRE(length == 0x1234 + constants::option_16_bit_offset);
        }
        SECTION("state machine")
        {
            coap::options::delta_length_decoder d;
            using c = coap::errc;

            c code = d.decode_byte(val1[0]);
            REQUIRE(code == c::again);
            code = d.decode_byte(val1[1]);
            REQUIRE(code == c::again);
            code = d.decode_byte(val1[2]);
            REQUIRE(code == c::done);

            REQUIRE(d.delta() == 1);
            REQUIRE(d.length() == 0x1234 + constants::option_16_bit_offset);
        }
    }
    SECTION("decoder")
    {
        using namespace coap::options;

        using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

        int counter = 0;
        bool has_payload;

        estd::detail::basic_ispanbuf<const uint8_t> in(test::op_data1);
        decoder_type decoder(test::op_data1);

        // compiles, but doesn't seem to call f()
        //decode_exp(in, [&](const auto& o)
        //estd::errc err = decode(estd::span(test::data1), [&](const auto& o)
        coap::errc err = decoder.decode_combined([&](const auto o)
        {
            // This is not great, but serviceable
            if constexpr(!is_constexpr<decltype(o)>)
            {
                option o2 = o;
            }
            else
            {
                // NOTE: clang doesn't like const auto&
                constexpr numbers number = o.number;

                if constexpr(number == numbers::UriHost)
                {
                    ++counter;
                    REQUIRE(o.string() == "host");
                }
                else if constexpr(number == numbers::UriPath)
                {
                    if(++counter == 2)
                        REQUIRE(o.string() == "v1");
                    else
                        REQUIRE(o.string() == "t");
                }
            }
        }, &has_payload);

        REQUIRE(err == coap::errc{});
        REQUIRE(counter == 3);
    }
    SECTION("stateful decoder")
    {
        using namespace coap::options;

        estd::detail::basic_ispanbuf<const uint8_t> in(test::op_data1);
        stateful_decoder decoder;
        int counter = 0;

        coap::errc err = decoder.decode(in, [&](option<> o)
            {
                switch(o.number)
                {
                    case numbers::UriHost:
                        ++counter;
                        REQUIRE(o.string() == "host");
                        break;

                    case numbers::UriPath:
                        ++counter;
                        break;

                    default:
                        FAIL("unexpected number: " << o.number);
                        break;
                }
            });

        REQUIRE(counter == 3);
        // NOTE: 'again' indicates payload present.  Otherwise we see
        // 'done'.  Kinda clulnky, since 'again' misleads one into thinking
        // more options might be available
        REQUIRE(err == coap::errc::again);
    }
    SECTION("option numbers dispatcher")
    {
        // TODO: Move this elsewhere
        using namespace coap::options;

        int counter = 0;

        dispatch_number(numbers::UriPath, [&](auto number)
        {
            //using traits = option_traits<number>;

            //static_assert(traits::number == numbers::UriPath);
            if constexpr(number == numbers::UriPath)
            {
                ++counter;
            }
        });

        REQUIRE(counter == 1);
    }
}
