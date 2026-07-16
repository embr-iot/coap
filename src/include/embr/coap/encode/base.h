#include "fwd.h"

namespace embr::coap::internal {

struct encoder_base
{
    // DEBT: Rename to 'states'
    enum States
    {
        Header,
        Token,
        Options,
        Payload
    };

#if !UNIT_TESTING
protected:
#endif

    States state_{Header};

public:
    constexpr States state() const { return state_; }
};

}
