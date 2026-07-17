#pragma once

#include "fwd.h"

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
    enum codes
    {
        Done,
        More,       ///<! More data expected
        Bad         ///<! Corrupt Data
    };

    codes decode_length();  // DEBT: Make internal
    codes decode_byte(uint8_t c);

    void reset();

    using modes = internal::option_enum_base::extended_modes;

    constexpr unsigned delta() const { return delta_; }
    constexpr unsigned length() const { return length_; }
};

}
