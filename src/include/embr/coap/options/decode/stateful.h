#pragma once

#include "delta-length-decoder.h"

#include <estd/system_error.h>

namespace embr::coap::options {

// Alternative to regular decoder<Streambuf>:
// - Fully resilient to streambuf data availability variance, non blocking
// - Much harder to use
class stateful_decoder
{
    delta_length_decoder dlc_{};
    uint16_t current_number_{};

    using errc = estd::errc;

    enum states
    {
        Header,
        Value
    };

    states state_{Header};

public:
    ///
    /// @brief decode_one
    /// @return
    ///     operation_in_progress = keep polling, normal operation
    ///     resource_unavailable_try_again = keep polling, normal operation
    ///     invalid_argument = bad data encountered
    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
    errc decode_one(Streambuf&, F&&);

    template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
    errc decode(Streambuf&, F&&);
};

}
