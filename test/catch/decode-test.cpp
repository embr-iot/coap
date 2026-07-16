#include "test-data.h"

#include <embr/coap/decoder.h>

#include <catch2/catch_all.hpp>

#include <estd/span.h>

using namespace embr::coap;

TEST_CASE("top-level decoding", "[decode]")
{
    using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

    decoder_type decoder(test::htop_data2);

    header h;
    auto h_expected = (const header*)test::h_data2;
    token t;

    decoder >> h;

    REQUIRE(decoder.good());
    REQUIRE(*h_expected == h);
    REQUIRE(*h_expected == decoder.header());

    decoder >> t;

    REQUIRE(decoder.good());

    REQUIRE(t.size == 2);
    REQUIRE(t.value[0] == 1);
    REQUIRE(t.value[1] == 2);

    REQUIRE(decoder.state() == decoder_type::Options);

    estd::string_view host{};

    estd::errc err = decoder.options_decode([&](const auto o)
        {
            if constexpr(o.number == options::numbers::UriHost)
            {
                host = o.string();
            }
        });

    REQUIRE(err == estd::errc{});
    REQUIRE(decoder.good());
    REQUIRE(host == "host");
    REQUIRE(decoder.state() == decoder_type::Payload);

    REQUIRE(decoder.in().in_avail() == 1);
    REQUIRE(*decoder.in().gptr() == 'x');
}

TEST_CASE("top-level decoding (stateful)", "[decode][stateful]")
{
    estd::detail::basic_ispanbuf<const uint8_t> in(test::htop_data2);
    stateful_decoder decoder;
    int counter = 0;

    decoder.poll_one(in, [&](auto state, auto param)
    {
        if constexpr(state == decoder.Header)
        {
            ++counter;
            const header& h = param;
        }
        else if constexpr(state == decoder.Token)
        {
            ++counter;
        }
        else if constexpr(state == decoder.Option)
        {
            ++counter;
            const options::option<>& o = param;
        }
    });

    REQUIRE(counter == 3);
}