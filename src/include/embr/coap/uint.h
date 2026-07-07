#pragma once

#include "stdlib.h"

#include <estd/cstdint.h>
#include <estd/limits.h>

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

// Encode an integer in big endian/network order.
template <typename Integer>
constexpr uint8_t* uint_encode(const uint8_t* const begin, uint8_t* end, Integer value)
{
    uint8_t* out = end - 1;

    while(out > begin)
    {
        *out-- = value & 0xFF;
        value >>= 8;
    }

    *out = value & 0xFF;

    return end;
}

}

template <typename Integer>
constexpr uint8_t* uint_encode(uint8_t* out, Integer value, const unsigned len)
{
    return internal::uint_encode(out, out + len, value);
}

template <typename Integer>
constexpr uint8_t* uint_encode(uint8_t* out, Integer value)
{
    //using limits = estd::numeric_limits<Integer>;

    if(value == 0)
        return out;
    else if(sizeof(Integer) == 1 || value <= 0xFF)
    {
        *out++ = value;
        return out;
    }
    else if(sizeof(Integer) == 2 || value <= 0xFFFF)
    {
        return uint_encode(out, value, 2);
    }
    else if(value <= 0xFFFFFF)
    {
        return uint_encode(out, value, 3);
    }
    else if(value <= 0xFFFFFFFF)
    {
        return uint_encode(out, value, 4);
    }

    abort();
}


}
