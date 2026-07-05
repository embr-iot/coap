#pragma once

#include <estd/cstdint.h>

#include "fwd.h"

namespace embr::coap {

namespace internal {

template <typename Return>
inline Return uint_get(const uint8_t* value, const unsigned len)
{
    // coap cleverly allows 0-length integer buffers, which means value=0
    if(len == 0) return 0;

    Return v = *value;

    for(unsigned i = 1; i < len; i++)
    {
        v <<= 8;
        v |= value[i];
    }

    return v;
}

}


}
