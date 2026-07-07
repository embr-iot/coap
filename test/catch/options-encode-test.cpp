#include <embr/coap/options/encode.h>

#include <catch2/catch_all.hpp>

#include <estd/span.h>

using namespace embr;

TEST_CASE("options encoding", "[encode][options]")
{
    uint8_t out[32]{};

    using numbers = coap::options::numbers;

    SECTION("option number + length")
    {
        SECTION("no extensions")
        {
            // Upper part = 11 (Uri) lower part = 10 (length)
            static constexpr uint8_t expected[] = { 0xBA };

            uint8_t* out2 = coap::options::delta_length_encode(out, (numbers)0, numbers::UriPath, 10);

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
}
