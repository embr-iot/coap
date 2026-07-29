#include "test-data.h"
#include "test-stream.h"

#include <embr/coap/encoder.h>

#include <estd/ostream.h>
#include <estd/span.h>
#include <estd/sstream.h>

#include <catch2/catch_all.hpp>

using namespace embr::coap;

TEST_CASE("top-level encoding", "[encode]")
{
    union
    {
        uint8_t out_uint8[32]{};
        char char_out[32];
    };

    SECTION("streambuf: spanbuf")
    {
        using encoder_type = encoder<estd::ospanbuf>;

        encoder_type encoder(char_out);

        encoder << header(header::NON, header::PUT);
        encoder << options::uri_path << "Hello";
        encoder << payload << "x";

        constexpr uint8_t expected[] { H_DATA3, 0xB5, 'H', 'e', 'l', 'l', 'o', 0xFF, 'x'};

        REQUIRE_THAT(estd::span(out_uint8, 12), Catch::Matchers::RangeEquals(estd::span(expected)));
    }
    SECTION("streambuf: stringbuf")
    {
        // stringbuf + coap has some issues.  See:
        // https://github.com/malachi-iot/estdlib/issues/224
        // https://github.com/malachi-iot/estdlib/issues/229
        using encoder_type = encoder<estd::layer2::basic_stringbuf<char, 32, false>>;

        encoder_type encoder(char_out);

        encoder << header(header::NON, header::PUT);
        /*
        encoder << options::uri_path << "Hello";
        encoder << payload << "x"; */

        REQUIRE_THAT(estd::span(char_out, 4), Catch::Matchers::RangeEquals(estd::span(test::h_data3)));
    }
    SECTION("stateful")
    {
        restrained_ospanbuf out(char_out);

        out.limiter = 2;

        uint8_t token[] { 1, 2, 3, 4, 5 };

        stateful_encoder encoder;

        bool r = encoder.header(out, header(header::CON, header::GET, 5, 0));
        REQUIRE(!r);
        r = encoder.header(out);
        REQUIRE(r);
        r = encoder.token(out, token, 5);
        REQUIRE(!r);
        r = encoder.token(out);
        REQUIRE(!r);
        r = encoder.token(out);

        REQUIRE(r);
        REQUIRE(out.pos() == 9);

        r = encoder.options().number_and_length(out, options::numbers::UriHost, 4);

        REQUIRE(r);
        REQUIRE(out.pos() == 10);

        // Manually dealing with unideal out-streambuf
        REQUIRE(out.sputn("host", 4) == 2);
        REQUIRE(out.sputn("st", 2) == 2);

        r = encoder.payload(out);

        REQUIRE(r);
        REQUIRE(out.pos() == 15);
    }
}