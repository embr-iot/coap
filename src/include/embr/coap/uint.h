#pragma once

#include "fwd.h"

#include <estd/cstdint.h>
#include <estd/cstdlib.h>
#include <estd/limits.h>

#include "assert.h"

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

// EXPERIMENTAL
// Bad idea to intermingle const and non-const like this
class uint
{
    union
    {
        const uint8_t* const_data_;
        uint8_t* data_;
    };
    unsigned len_;

public:
    constexpr uint(uint8_t* data, unsigned len) :
        data_(data), len_(len)
    {
    }

    constexpr uint(const uint8_t* data, unsigned len) :
        const_data_(data), len_(len)
    {
    }

    template <typename Integer>
    [[nodiscard]] constexpr Integer decode() const
    {
        return uint_decode<Integer>(data_, len_);
    }

    template <typename Integer>
    void encode(Integer value)
    {
        // Experimental, rewriting max length with actual encoded length
        uint8_t* out = uint_encode(data_, data_ + len_, value);
        len_ = out - data_;
    }

    constexpr operator unsigned() const
    {
        return decode<unsigned>();
    }

    uint& operator=(unsigned v)
    {
        encode(v);
        return *this;
    }
};


}
