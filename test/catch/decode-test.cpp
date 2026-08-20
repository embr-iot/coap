#include "test-data.h"

#include <embr/coap/decoder.h>

#include <embr/coap/decode/breadcrumb.h>

#include <catch2/catch_all.hpp>

#include <estd/span.h>

using namespace embr::coap;

static_assert(internal::policies_enum::deduce_in<estd::ispanbuf>() ==
    internal::policies_enum::Presumptive);
static_assert(header::from(test::h_data3).code() == header::PUT);

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

        REQUIRE(opt.number == options::numbers::UriHost);
        REQUIRE(opt.string() == "host");

        REQUIRE(decoder.state() == decoder_type::Payload);
    }
}

TEST_CASE("top-level decoding: DATA3", "[decode]")
{
    using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

    decoder_type decoder(test::htop_data3);

    header h;
    token t;

    decoder >> h;
    decoder >> t;

    options::option opt;

    decoder >> opt;

    REQUIRE(opt.number == options::numbers::UriHost);

    REQUIRE(decoder.state() == decoder_type::Done);
}

namespace ids = test::ids;

using bc = embr::internal::breadcrumb;

static constexpr bc nav_data1[]
{
    { "v1",     ids::v1 },
    { "t",      ids::v1_t,      ids::v1 },
    { "v2",     ids::v2 },
    { "yes",    ids::v2_yes,    ids::v2 },
    { "id",     ids::v2_yes_id, ids::v2_yes },
    bc::null()
};

TEST_CASE("top-level decoding: breadcrumb (DATA4)", "[decode]")
{
    using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

    decoder_type decoder(test::htop_data4);

    header h;
    token t;

    decoder >> h;
    decoder >> t;   // DEBT: Make reading token optional so that if we skip it below decoder >> opt doesn't flip out

    REQUIRE(decoder.good());

    SECTION("options bit by bit")
    {
        embr::breadcrumb_matcher match(nav_data1);

        options::option opt;

        decoder >> opt;

        REQUIRE(opt.number == options::numbers::UriHost);

        do
        {
            decoder >> opt;

            REQUIRE(decoder);

            match.search(opt.string());
        }
        while(decoder.state() == decoder_type::Options);

        REQUIRE(decoder.state() == decoder_type::Payload);
        REQUIRE(match.current()->id == ids::v2_yes_id);
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