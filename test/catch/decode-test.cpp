#include "test-data.h"

#include <embr/coap/decode.h>

#include <catch2/catch_all.hpp>

#include <estd/span.h>

using namespace embr::coap;

TEST_CASE("top-level decoding", "[decode]")
{
    using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

    decoder_type decoder(test::htop_data1);

    header h;
    token t;

    decoder >> h;

    REQUIRE(decoder.good());

    decoder >> t;

    REQUIRE(decoder.good());

    REQUIRE(t.size == 2);
    REQUIRE(t.value[0] == 1);
    REQUIRE(t.value[1] == 2);

    REQUIRE(decoder.state() == decoder_type::Options);

    estd::string_view host{};

    decoder.options().decode([&](const auto o)
        {
            if constexpr(o.number == options::numbers::UriHost)
            {
                host = o.string();
            }
        });

    REQUIRE(host == "host");
}
