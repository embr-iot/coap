#pragma once

#include "delta-length-decoder.h"

namespace embr::coap::options {

// DEBT: Be aware no extra help during retry (maybe you want a pubsync, delay, etc)
// otherwise we would have used the more direct delta_length_decode call
/// @returns
///     nullopt - happily finished decoding
///     char_traits::eof - bad stream OR eof discovered (DEBT, disambiguate if we can)
///     -2 - bad data during decode
///     0xFF - payload discovered
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
estd::optional<int> delta_length_decode(delta_length_decoder& dld, Streambuf& in)
{
    using char_traits = typename Streambuf::traits_type;

    auto valid = [](int c)
    {
        // Be advised, bug https://github.com/malachi-iot/estdlib/issues/220
        // presents 0xFF AS -1
        // DEBT: End-of-current-data not same as EOL or maybe EOF
        return c != 0xFF && char_traits::not_eof(c);
    };

    int c;

    while(valid(c = in.sbumpc()))
    {
        switch(dld.decode_byte(c))
        {
            case errc::done:
                // NOTE: Caller must consume value portion from streambuf themself including
                // advancing streambuf forward
                return {};

            case errc::bad:
                // DEBT: Need better indicator
                // EOF isn't guaranteed to be -1 (and therefore not -2) from streambuf
                return -2;

            case errc::again:   break;

            default:
                abort();
        }
    }

    return c;
}

// DEBT: Be aware no extra help during retry (maybe you want a pubsync, delay, etc)
// otherwise we would have used the more direct delta_length_decode call
/// @returns pass through return value from above
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::optional<int> delta_length_decode(Streambuf& in, F&& f)
{
    delta_length_decoder dld;

    estd::optional<int> c = delta_length_decode(dld, in);

    // DEBT: Pretty sure we should be able to compare against nullopt too
    if(!c.has_value()) f(dld);

    return c;
}


}