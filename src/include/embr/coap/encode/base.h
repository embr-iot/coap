#pragma once

#include "fwd.h"

namespace embr::coap::internal {

struct encoder_base
{
    // DEBT: Rename to 'states'
    enum states
    {
        Header,
        Token,
        Options,
        Payload
    };

#if !UNIT_TESTING
protected:
#endif

    states state_{Header};

public:
    [[nodiscard]] constexpr states state() const { return state_; }
};

}
