#pragma once

#include "../../internal/errc.h"
#include "delta-length-decoder.h"

#include <estd/system_error.h>

namespace embr::coap::options {

// Alternative to regular decoder<Streambuf>:
// - Fully resilient to streambuf data availability variance, non blocking
// - Much harder to use
class stateful_decoder
{
    delta_length_decoder dld_{};
    uint16_t current_number_{};

    // NOT READY YET
    // Default mode is to piece out option value on your behalf, basically presenting
    // you 'opaque' as a sliding window through value.  You may turn that off in which
    // case YOU must advance the stream yourself by o.length.
    bool auto_chunk_{true};

    enum states
    {
        Header,
        Value
    };

    states state_{Header};

public:
    stateful_decoder() = default;
    explicit stateful_decoder(uint16_t current_number) :
        current_number_{current_number}
    {}

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

    constexpr uint16_t current_number() const { return current_number_; }
};

}
