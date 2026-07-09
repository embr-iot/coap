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
            encoder <<
                coap::options::uri_host << "host" <<
                coap::options::uri_path << "v1" << "thing";

            REQUIRE(out_size() == 14);

            SECTION("then payload")
            {
                encoder_type::payload_type test = encoder << coap::payload;

                // DEBT: https://github.com/malachi-iot/estdlib/issues/218 poop
                test.setf(estd::ios_base::uppercase);
                test.write("123", 3);
                test << "xyz" << estd::hex << 15;

                REQUIRE(out_size() == 22);
                REQUIRE(std::string_view(char_out + 15, 7) == "123xyzF");
            }
        }
    }
}
