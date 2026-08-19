#include "test-data.h"

#include <embr/coap/internal/constants.h>
#include <embr/coap/options/decode.h>
#include <embr/coap/options/numbers.h>

#include <cstring>  // DEBT: breadcrumb needs this
#include <embr/internal/breadcrumb.h>

#include <catch2/catch_all.hpp>

using namespace embr;

using bc = embr::internal::breadcrumb;

namespace ids {

enum nav_data1 : int
{
    v1,
    v1_t
};

}

static constexpr bc nav_data1[]
{
    { "v1",  ids::v1 },
    { "t",   ids::v1_t, ids::v1 },
    bc::null()
};

template <class Breadcrumb = embr::internal::breadcrumb>
class breadcrum_helper
{
    using bc = Breadcrumb;

    const bc* top_;
    const bc* current_;

    using traits = embr::internal::breadcrumb_traits<bc>;

public:
    constexpr breadcrum_helper(const bc* top) : top_{top}, current_{} {}

    constexpr bool at_end() const { return current_ && current_->id == -1; }

    const bc* current() const { return current_; }

    // Keep this in the helper to avoid ADL things
    template <class String>
    ESTD_CPP_CONSTEXPR(17) static const bc* search(const String& v, const bc* top, const bc* current)
    {
        // Just starting out, pretend to have root node so search siblings without a child
        // descent
        if(current == nullptr)  return internal::search_siblings(top, v);

        if(current->id == -1)  // End marker
            return nullptr;

        // DEBT: I believe first_child implicitly checks for end marker too, but don't
        // rely on that just yet until embr has unit tests to prove it
        const bc* child = first_child(current);

        if(child)   return search_siblings(child, v);

        return current;
    }

    template <class String>
    ESTD_CPP_CONSTEXPR(17) const bc* search(const String& v)
    {
        return current_ = search(v, top_, current_);
    }
};

TEST_CASE("options decoding", "[decode][options]")
{
    SECTION("numbers")
    {
        constexpr uint8_t val1[] { 0x1E, 0x12, 0x34 };
        coap::options::numbers number;
        namespace constants = coap::constants;
        unsigned length{3};

        SECTION("direct")
        {
            const uint8_t* out1 = coap::options::delta_length_decode(val1, 0, &number, &length);

            REQUIRE(out1 == val1 + 3);
            REQUIRE(number == 1);
            REQUIRE(length == 0x1234 + constants::option_16_bit_offset);
        }
        SECTION("state machine")
        {
            coap::options::delta_length_decoder d;
            using c = coap::errc;

            c code = d.decode_byte(val1[0]);
            REQUIRE(code == c::again);
            code = d.decode_byte(val1[1]);
            REQUIRE(code == c::again);
            code = d.decode_byte(val1[2]);
            REQUIRE(code == c::done);

            REQUIRE(d.delta() == 1);
            REQUIRE(d.length() == 0x1234 + constants::option_16_bit_offset);
        }
    }
    SECTION("decoder: dispatch")
    {
        using namespace coap::options;

        using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

        int counter = 0;
        bool has_payload;

        decoder_type decoder(test::op_data1);

        breadcrum_helper bch(nav_data1);

        coap::errc err = decoder.dispatch_combined([&](const auto o)
        {
            // This is not great, but serviceable
            if constexpr(!is_constexpr<decltype(o)>)
            {
                option o2 = o;
            }
            else
            {
                // NOTE: clang doesn't like const auto&
                constexpr numbers number = o.number;

                if constexpr(number == numbers::UriHost)
                {
                    ++counter;
                    REQUIRE(o.string() == "host");
                }
                else if constexpr(number == numbers::UriPath)
                {
                    // In the real world, top-level (nav_data1) you won't issue 'child' on
                    // FIX: If a rogue o.string() comes along, search returns nullptr, which is OK,
                    // but will then confuse this logic into going back to root which might create
                    // a false match
                    //uri_path = search(uri_path ? child(uri_path) : child(nav_data1), o.string());
                    const bc* uri_path = bch.search(o.string());

                    if(++counter == 2)
                    {
                        REQUIRE(o.string() == "v1");
                        REQUIRE(uri_path);
                        REQUIRE(uri_path->id == ids::v1);
                    }
                    else
                    {
                        REQUIRE(o.string() == "t");
                        REQUIRE(uri_path);
                        REQUIRE(uri_path->id == ids::v1_t);
                    }
                }
            }
        }, &has_payload);

        REQUIRE(err == coap::errc{});
        REQUIRE(counter == 3);
        REQUIRE(bch.current()->id == ids::v1_t);
    }
    SECTION("decoder: decode_one")
    {
        using namespace coap::options;

        using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

        int counter = 0;
        uint16_t current_number = 0;
        option<> opt{};

        SECTION("data#1")
        {
            decoder_type decoder(test::op_data1);

            coap::errc err = decoder.decode_one(&opt, &current_number);

            REQUIRE(err == coap::errc{});
            REQUIRE(opt.number == numbers::UriHost);

            err = decoder.decode_one(&opt, &current_number);

            REQUIRE(err == coap::errc{});
            REQUIRE(opt.number == numbers::UriPath);
            REQUIRE(opt.string() == "v1");

            err = decoder.decode_one(&opt, &current_number);

            REQUIRE(err == coap::errc{});
            REQUIRE(opt.number == numbers::UriPath);
            REQUIRE(opt.string() == "t");
        }
        SECTION("data#2")
        {
            decoder_type decoder(test::op_data2);

            coap::errc err = decoder.decode_one(&opt, &current_number);

            REQUIRE(err == coap::errc{});
            REQUIRE(opt.number == numbers::UriHost);

            err = decoder.decode_one(&opt, &current_number);

            // For low-level-ish decode_one, 'alternate' means Payload encountered
            REQUIRE(err == coap::errc::alternate);
        }
    }
    SECTION("stateful decoder")
    {
        using namespace coap::options;

        estd::detail::basic_ispanbuf<const uint8_t> in(test::op_data1);
        stateful_decoder decoder;
        int counter = 0;

        coap::errc err = decoder.decode(in, [&](option<> o)
            {
                switch(o.number)
                {
                    case numbers::UriHost:
                        ++counter;
                        REQUIRE(o.string() == "host");
                        break;

                    case numbers::UriPath:
                        ++counter;
                        break;

                    default:
                        FAIL("unexpected number: " << o.number);
                        break;
                }
            });

        REQUIRE(counter == 3);
        // NOTE: 'again' indicates payload present.  Otherwise we see
        // 'done'.  Kinda clulnky, since 'again' misleads one into thinking
        // more options might be available
        REQUIRE(err == coap::errc::again);
    }
    SECTION("option numbers dispatcher")
    {
        // TODO: Move this elsewhere
        using namespace coap::options;

        int counter = 0;

        SECTION("primary case")
        {
            dispatch_number(numbers::UriPath, [&](auto number)
            {
                //using traits = option_traits<number>;

                //static_assert(traits::number == numbers::UriPath);
                if constexpr(number == numbers::UriPath)
                {
                    ++counter;
                }
            });

            REQUIRE(counter == 1);
        }
        SECTION("helper options.dispatch method")
        {
            option<> o(numbers::UriHost, 4, "host");

            bool r = o.dispatch([&](const auto o2)
            {
                if constexpr(o2.number == numbers::UriHost)
                {
                    ++counter;
                    if(o2.string() == "host")   ++counter;
                }
            });

            REQUIRE(r);
            REQUIRE(counter == 2);
        }
    }
}
