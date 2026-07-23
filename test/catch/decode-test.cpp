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

    SECTION("options decode callback")
    {
        errc err = decoder.options_decode([&](const auto o)
            {
                if constexpr(o.number == options::numbers::UriHost)
                {
                    host = o.string();
                }
            });

        REQUIRE(err == errc{});
        REQUIRE(decoder.good());
        REQUIRE(host == "host");
        REQUIRE(decoder.state() == decoder_type::Payload);

        REQUIRE(decoder.in().in_avail() == 1);
        REQUIRE(*decoder.in().gptr() == 'x');
    }
    SECTION("options bit by bit")
    {
        options::option opt;

        decoder >> opt;

        // NOTE: Partially works, but value part not quite sorted out yet.  See operator>>
        REQUIRE(opt.number == options::numbers::UriHost);

        //REQUIRE(decoder.state() == decoder_type::Payload);
    }
}

TEST_CASE("top-level decoding", "[decode][char]")
{
    using stream_type = estd::detail::basic_ispanstream<const char>;
    stream_type in((const char*)test::htop_data2, sizeof(test::htop_data2));
    using streambuf_type = stream_type::streambuf_type;
    using decoder_type = decoder<streambuf_type&>;
    decoder_type decoder(*in.rdbuf());

    header h;
    auto h_expected = (const header*)test::h_data2;
    token t;

    decoder >> h;
    decoder >> t;

    REQUIRE(decoder.good());

    estd::string_view host{};

    errc err = decoder.options_decode([&](const auto o)
        {
            if constexpr(o.number == options::numbers::UriHost)
            {
                host = o.string();
            }
        });

    // char streambuf doesn't yield us 0xFF as expected (a bug)
    // https://github.com/malachi-iot/estdlib/issues/220
    /*
    REQUIRE(decoder.state() == decoder_type::Payload);

    estd::layer1::string<32> payload;

    in >> payload;

    REQUIRE(payload == "x"); */
}

TEST_CASE("top-level decoding (stateful)", "[decode][stateful]")
{
    estd::detail::basic_ispanbuf<const uint8_t> in(test::htop_data2);
    stateful_decoder decoder;
    int counter = 0;
    errc err;

    SECTION("discrete")
    {
        header h{};
        token t;

        err = decoder.poll_one(in, &h);

        REQUIRE(err == errc{});

        err = decoder.poll_one(in, &t);

        REQUIRE(err == errc{});
        REQUIRE(t.value[0] == 1);
        REQUIRE(t.value[1] == 2);
    }
    SECTION("unified poll_one")
    {
        err = decoder.poll_one(in, [&](auto state, const auto& param)
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
    SECTION("sanity check with char-based streambuf")
    {
        estd::detail::basic_ispanstream<const char> in((const char*)test::htop_data2, sizeof(test::htop_data2));

    }
}