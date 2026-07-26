#pragma once

#include <estd/cstdint.h>

#include "assert.h"

namespace embr::coap {

namespace internal {

/// Encode an integer in big endian/network order (low level call).
template <typename Integer>
constexpr void uint_encode_fixed(const uint8_t* const begin, uint8_t* out, Integer value)
{
    for(;out > begin; value >>= 8)  *--out = value & 0xFF;
}

template <typename Integer>
constexpr uint8_t* uint_encode_deduced(uint8_t* begin, const uint8_t* const end, Integer value)
{
    assert(end - begin >= sizeof(value));

    // Internal call - 0 not supported
    assert(value > 0);

    uint8_t* out = begin;
    int shift = (sizeof(value) - 1) * 8;

    // Skip leading zero bytes
    for(; value >> shift == 0; shift -= 8);

    // DEBT: Consider dogfooding in uint_encode_fixed:
    // 1. Theoretically faster due to simpler bit shift (practically maybe not)
    // 2. Dogfooding obvious benefits: better code quality
    for(; shift >= 0; shift -= 8)
        *out++ = (value >> shift) & 0xFF;

    return out;
}

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
