#pragma once

#include "delta-length-decoder.h"

namespace embr::coap::options {

// DEBT: Be aware no extra help during retry (maybe you want a pubsync, delay, etc)
// otherwise we would have used the more direct delta_length_decode call
// EXPERIMENTAL, not used yet - but shaping up
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::optional<int> delta_length_decode(Streambuf& in, F&& f)
{
    delta_length_decoder dld;

    auto valid = [](int c)
    {
        // Be advised, bug https://github.com/malachi-iot/estdlib/issues/220
        // presents 0xFF AS -1
        // DEBT: End-of-current-data not same as EOL or maybe EOF
        return c != 0xFF && c != -1;
    };

    int c;

    while(valid(c = in.sbumpc()))
    {
        switch(dld.decode_byte(c))
        {
            case errc::done:
                f(dld);
                // NOTE: Caller must consume value portion from streambuf themself including
                // advancing streambuf forward
                return {};

            case errc::bad:
                // DEBT: Need better indicator, this will get misinterpreted as EOF and also
                // EOF isn't guaranteed to be -1 from streambuf
                return -1;

            case errc::again:   break;

            default:
                abort();
        }
    }

    return c;
}


}