#pragma once

#include "fwd.h"

#include "../../internal/accumulator.h"
#include "../../uint.h"

#include <estd/cstdint.h>
#include <estd/optional.h>
#include <estd/string_view.h>

namespace embr::coap::options {

/// @brief The stateful_encoder class
/// Usage by format:
/// 1. empty:           number_and_length with 0 length
/// 2. opaque/string:   number_and_length with manual subsequent streambuf calls
/// 3. uint:            number_and_uint (auto populates length)
class stateful_encoder
{
#if UNIT_TESTING
public:
#endif
    uint16_t current_number_;

    // 5 byte total,
    // 4-bit width for size tracking
    // true = ext activated, reducing width 3-bit
    internal::out_accumulator<5, 4, true> temp_;

    // Fill temp_ with encoded option header portion
    void number_and_length(numbers n, unsigned length);

public:
    stateful_encoder() = default;
    explicit stateful_encoder(estd::nullopt_t) :
        current_number_{},
        temp_{}
    {}

    // NOTE: Potentially phasing out in favor of 'poll_one' below
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_length(Streambuf& out)  { return temp_.sputn(out); }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_length(Streambuf& out, numbers n, unsigned length)
    {
        number_and_length(n, length);
        return temp_.sputn(out);
    }

    // EXPERIMENTAL, UNFINISHED
    // Depends on the 'ext' buffer of accumulator, meaning 's' lifetime
    // must last the duration of this option output (like string literals)
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_string(Streambuf& out, numbers n, estd::string_view s)
    {
        //using char_type = typename Streambuf::char_type;

        number_and_length(n, s.size());
        if(temp_.sputn(out))
        {
            // move straight to ext buffer portion
            temp_.init_ext(s.data(), s.size());
            return temp_.sputn_ext(out);
        }
        return false;
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf, class Unsigned>
    bool number_and_uint(Streambuf& out, numbers n, Unsigned v)
    {
        // We cleverly cheat and stuff in a 0, knowing that we can rewrite
        // without a complex re-encode since no uint options are more than 4.
        // this also means our current temp buffer of 5 is always big enough
        number_and_length(n, 0);

        if(v == 0) return true;

        uint8_t* start = temp_.buf_ + temp_.size();
        const uint8_t* end = uint_encode(start, v);
        // resize up accumulator to include encoded uint
        temp_.init(end - temp_.buf_);
        // update option header length portion with discovered size
        temp_.buf_[0] |= end - start;
        return temp_.sputn(out);
    }

    // NOTE: Potentially phasing out in favor of 'poll_one' below
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf, class Unsigned>
    bool number_and_uint(Streambuf& out)   { return temp_.sputn(out); }

    // EXPERIMENTAL
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool poll_one(Streambuf& out)
    {
        return temp_.is_ext() ? temp_.sputn_ext(out) : temp_.sputn(out);
    }
};

}