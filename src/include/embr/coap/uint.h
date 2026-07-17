#pragma once

#include "stdlib.h"

#include <estd/cstdint.h>
#include <estd/limits.h>

#include "fwd.h"

namespace embr::coap {

namespace internal {

template <typename Integer>
constexpr Integer uint_decode(const uint8_t* in, const uint8_t* end)
{
    // since CoAP uints have an implicit value of 0 (when len == 0), proactively
    // init to 0 rather than doing an len == 0 check every time
    Integer v{0};

    for(;in < end; ++in)
    {
        v <<= 8;
        v |= *in;
    }

    return v;
}

/// Encode an integer in big endian/network order (low level call).
/// @param out = begin + len - 1
template <typename Integer>
constexpr void uint_encode(const uint8_t* const begin, uint8_t* out, Integer value)
{
    while(out > begin)
    {
        *out-- = value & 0xFF;
        value >>= 8;
    }

    *out = value & 0xFF;
}

}

template <typename Integer = unsigned>
constexpr Integer uint_decode(const uint8_t* in, const unsigned len)
{
    return internal::uint_decode<Integer>(in, in + len);
}

template <typename Integer, typename Byte, size_t N>
constexpr Integer uint_decode(Byte (&in)[N])
{
    static_assert(sizeof(Byte) == 1);

    return internal::uint_decode<Integer>((const uint8_t*)in, in + N);
}


template <typename Integer>
constexpr uint8_t* uint_encode(uint8_t* out, Integer value, const unsigned len)
{
    uint8_t* end = out + len;
    internal::uint_encode(out, end - 1, value);
    return end;
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
