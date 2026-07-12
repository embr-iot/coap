#include "test-data.h"

#include <embr/coap/decode.h>

#include <catch2/catch_all.hpp>

#include <estd/span.h>

using namespace embr::coap;

TEST_CASE("top-level decoding", "[decode]")
{
    using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

    decoder_type decoder(test::op_data1);

    header h;

    decoder >> h;
}
