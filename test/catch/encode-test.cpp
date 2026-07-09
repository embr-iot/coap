#include <catch2/catch_all.hpp>

#include <embr/coap/encode.h>

#include <estd/span.h>

using namespace embr;

TEST_CASE("top-level encoding", "[encode]")
{
    union
    {
        uint8_t out[32]{};
        char char_out[32];
    };

    using encoder_type = coap::encoder<estd::ospanbuf>;

    encoder_type encoder(char_out);

    encoder << coap::header(coap::header::NON, coap::header::PUT);
    encoder << coap::options::uri_path << "Hello";
    encoder << coap::payload << "x";

    constexpr uint8_t expected[] {};
}