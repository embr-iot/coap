#pragma once

#include "fwd.h"

#include <estd/algorithm.h>
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

namespace internal {

// I have a feeling std has something like this already
template <typename T>
class deep_compare
{
protected:
    T* data_;
    unsigned len_;

    constexpr deep_compare(T* data, unsigned len) :
        data_(data), len_(len)
    {
    }

public:
    constexpr T* data() const { return data_; }
    constexpr unsigned size() const { return len_; }

    constexpr bool operator==(const deep_compare& compare_to) const
    {
        // DEBT: Use estd once https://github.com/malachi-iot/estdlib/issues/221 is implemented
        return std::equal(
            data_, data_ + len_,
            compare_to.data_,
            compare_to.data_ + compare_to.len_);
    }
};

template <typename Byte>
class uint : public deep_compare<Byte>
{
    using base_type = deep_compare<Byte>;
    using base_type::data_;
    using base_type::len_;

public:
    constexpr uint(Byte* data, unsigned len) :
        base_type(data, len)
    {
    }

    template <unsigned N>
    constexpr uint(Byte (&in)[N]) : uint(in, N) {}

    template <typename Integer>
    [[nodiscard]] constexpr Integer decode() const
    {
        return coap::uint_decode<Integer>(data_, len_);
    }

    template <typename Integer>
    void encode(Integer value)
    {
        // Experimental, rewriting max length with actual encoded length
        uint8_t* out = coap::uint_encode(data_, data_ + len_, value);
        len_ = out - data_;
    }

    // DEBT: If c++20, do a requires/constraint to accept a wider range of unsigned
    constexpr operator unsigned() const
    {
        return decode<unsigned>();
    }

    template <class T>
    estd::enable_if_t<estd::numeric_limits<T>::is_integer, uint&> operator=(T v)
    {
        encode(v);
        return *this;
    }
};

}

using uint = internal::uint<uint8_t>;
using const_uint = internal::uint<const uint8_t>;

}
