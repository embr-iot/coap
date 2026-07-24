#pragma once

#include "fwd.h"

#include <estd/cstdint.h>
#include <estd/limits.h>

#include "assert.h"
#include "stdlib.h"

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
template <typename Integer>
constexpr void uint_encode_fixed(const uint8_t* const begin, uint8_t* out, Integer value)
{
    for(;out > begin; value >>= 8)
        *--out = value & 0xFF;

    // Perhaps skipping the decrement makes things go faster?  Or would it cache better without it... ?
    *out = value & 0xFF;
}

template <typename Integer>
constexpr uint8_t* uint_encode_deduced(uint8_t* begin, const uint8_t* const end, Integer value)
{
    assert(end - begin >= sizeof(value));

    // Internal call - 0 not supported
    assert(value > 0);

    uint8_t* out = begin;
    int shift = (sizeof(value) - 1) * 8;

    // Skip leading zeroes
    for(; value >> shift == 0; shift -= 8);

    for(; shift >= 0; shift -= 8)
        *out++ = (value >> shift) & 0xFF;

    return out;
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

using internal::uint_encode_fixed;

template <typename Integer>
constexpr uint8_t* uint_encode(uint8_t* out, const uint8_t* const end, Integer value)
{
    //using limits = estd::numeric_limits<Integer>;

    if(value == 0)
        return out;
    else if constexpr(sizeof(Integer) == 1)
    {
        *out++ = value;
        return out;
    }

    return internal::uint_encode_deduced(out, end, value);
}


}
