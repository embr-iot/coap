#pragma once

#include "../header.h"
#include "../header/token.h"
#include "../options/decode.h"
#include "../internal/errc.h"

namespace embr::coap {

class stateful_decoder
{
    options::stateful_decoder options_;

public:
    using errc = estd::errc;

    // DEBT: Probably both encoders and both decoders can all share one states enum
    enum states : uint8_t
    {
        Header,
        Token,
        Option,
        Options = Option,
        Payload,
        Done
    };

private:
    states state_{Header};

    // TODO: Looks like we could mildly refactor out_accumulator to assist in this
    uint8_t accumulator_[8];
    uint8_t pos_{};
    uint8_t size_{};

    // accumulator interface
    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
    errc sgetn(Streambuf& in, uint8_t*, unsigned sz);

    // contiguous optimistic flavor
    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
    errc sgetn_exp(Streambuf& in, uint8_t**, unsigned sz);

    // All-in-one approach
    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
    errc poll_one_ll(Streambuf& in, F&&);

public:
    // Piecemeal approach
    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
    errc poll_one(Streambuf& in, header*);

    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
    errc poll_one(Streambuf& in, token*);

    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
    errc poll_one(Streambuf& in, options::option<>*);

    options::stateful_decoder& options()
    {
        return options_;
    }

    // All-in-one approach
    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
    errc poll_one(Streambuf& in, F&&);


    // Payload you gotta do yourself once options() tells you to!
};

}
