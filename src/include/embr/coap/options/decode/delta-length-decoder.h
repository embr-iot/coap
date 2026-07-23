#pragma once

#include "fwd.h"

#include "../../internal/errc.h"

namespace embr::coap::options {

// State machine flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
class delta_length_decoder
{
    friend class stateful_decoder;

    enum states : uint8_t
    {
        Header,
        Delta1,
        Delta2,
        Length1,
        Length2,
    };

    states state_{Header};

    // Technically CoAP supports delta/length > 65536 - but that isn't happening
    uint16_t delta_;
    uint16_t length_;

public:
    errc decode_length();  // DEBT: Make internal

    ///
    /// @returns
    ///     done: obviously, finished
    ///     again: more data required
    ///     bad: unexpected data encountered
    errc decode_byte(uint8_t c);

    void reset();

    using modes = internal::option_enum_base::extended_modes;

    constexpr unsigned delta() const { return delta_; }
    constexpr unsigned length() const { return length_; }
};

}
