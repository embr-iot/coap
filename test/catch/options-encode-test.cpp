#include "test-data.h"

#include <embr/coap/encode.h>
#include <embr/coap/options/encode.h>

#include <catch2/catch_all.hpp>

#include <estd/span.h>

using namespace embr;

TEST_CASE("options encoding", "[encode][options]")
{
    union
    {
        uint8_t out[32]{};
        char char_out[32];
    };

    using numbers = coap::options::numbers;

    SECTION("option number + length")
    {
        SECTION("no extensions")
        {
            // Upper part = 11 (Uri) lower part = 10 (length)
            static constexpr uint8_t expected[] = { 0xBA };

            uint8_t* out2 = coap::options::delta_length_encode(out, 0, numbers::UriPath, 10);

            REQUIRE(out2 == out + 1);
            REQUIRE(*out == expected[0]);
        }
        SECTION("16-bit extended, both")
        {
            // 0xEE means 16-bit extended indicator is set for both option number and length
            static constexpr uint8_t expected[] = { 0xEE, 00, numbers::RequestTag - 269, 00, 400 - 269 };
            uint8_t* out2 = coap::options::delta_length_encode(out, 0, numbers::RequestTag, 400);

            REQUIRE(out2 == out + 5);

            REQUIRE_THAT(estd::span(out, 5), Catch::Matchers::RangeEquals(expected));
        }
    }
    SECTION("encoder")
    {
        //using encoder_type = coap::options::encoder<estd::detail::basic_ospanbuf<uint8_t>>;
        using encoder_type = coap::options::encoder<estd::ospanbuf>;

        encoder_type encoder(char_out);

        // DEBT: Use pubseekoff reporting since it's more standard
        auto out_size = [&] { return encoder.out().pos(); };

        SECTION("basic uri")
        {
            encoder << coap::options::uri_path << "Hello";

            REQUIRE(out_size() == 6);
        }
        SECTION("complex uri + host")
        {
            constexpr uint8_t expected[] = { 0x34, 'h', 'o', 's', 't', 0x82, 'v', '1', 0x05, 't' };

            encoder <<
                coap::options::uri_host << "host" <<
                coap::options::uri_path << "v1" << "thing";

            REQUIRE(out_size() == 14);

            REQUIRE_THAT(estd::span(out, sizeof(expected)), Catch::Matchers::RangeEquals(estd::span(expected)));

            /* temporarily disabled, needs 'child_encoder'
            SECTION("then payload")
            {
                encoder_type::payload_type test = encoder << coap::payload;

                // DEBT: https://github.com/malachi-iot/estdlib/issues/218 poop
                test.setf(estd::ios_base::uppercase);
                test.write("123", 3);
                test << "xyz" << estd::hex << 15;

                REQUIRE(out_size() == 22);
                REQUIRE(std::string_view(char_out + 15, 7) == "123xyzF");
            }   */
        }
    }
    SECTION("child_encoder")
    {
        using namespace coap;
        namespace o = options;

        using encoder_type = coap::encoder<estd::ospanbuf>;
        using options_encoder_type = typename encoder_type::options_encoder_type;

        encoder_type encoder(char_out);

        // Synthetically brute force top-level encoder into options mode to satisfy
        // state machine validation
        encoder.state_ = encoder_type::Options;

        options_encoder_type options_encoder(encoder);

        options_encoder << o::uri_host << "host" << o::uri_path << "v1" << "t";
        options_encoder << payload << "x";

        REQUIRE_THAT(estd::span(out, sizeof(test::op_data1)), Catch::Matchers::RangeEquals(estd::span(test::op_data1)));
    }
    SECTION("stateful")
    {
        using namespace coap::options;

        estd::ospanbuf out(char_out);

        stateful_encoder encoder(estd::nullopt);

        bool b = encoder.number_and_length(out, numbers::UriPath, 8);

        REQUIRE(b);
        REQUIRE(encoder.temp_.size() == 1);
        REQUIRE(out.pos() == 1);

        out.sputn("pathname", 8);

        REQUIRE(out.pos() == 9);

        b = encoder.number_and_uint(out, numbers::MaxAge, 0x123);

        REQUIRE(b);
        REQUIRE(out.pos() == 12);
    }
}
