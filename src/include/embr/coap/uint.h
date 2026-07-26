#pragma once

#include "fwd.h"
#include "uint/compare.h"
#include "uint/decode.h"
#include "uint/encode.h"

#include <estd/algorithm.h>
#include <estd/cstdint.h>
#include <estd/cstdlib.h>
#include <estd/limits.h>

#include "assert.h"
#if __cpp_impl_three_way_comparison
#include <compare>
#endif

namespace embr::coap {


namespace internal {

// I have a feeling std has something like this already
// Note the similarity to out_accumulator
template <typename T, unsigned width = 16>
class deep_compare
{
protected:
    T* data_;
    unsigned size_ : width;
    unsigned max_size_ : width;

    constexpr deep_compare(T* data, unsigned len) :
        data_(data), size_(len), max_size_(len)
    {
    }

public:
    constexpr T* data() const { return data_; }
    constexpr unsigned size() const { return size_; }

    // NOTE: This turns out to not be reliable for uint due to leading zeroes
    constexpr bool operator==(const deep_compare& compare_to) const
    {
        // DEBT: Use estd once https://github.com/malachi-iot/estdlib/issues/221 is implemented
        return std::equal(
            data_, data_ + size_,
            compare_to.data_,
            compare_to.data_ + compare_to.size_);
    }
};

template <typename Byte>
class uint : public deep_compare<Byte>
{
    using base_type = deep_compare<Byte>;
    using base_type::data_;
    using base_type::size_;

public:
    constexpr uint(Byte* data, unsigned size) :
        base_type(data, size)
    {
    }

    template <unsigned N>
    constexpr uint(Byte (&in)[N]) : uint(in, N) {}

    template <typename Integer>
    [[nodiscard]] constexpr Integer decode() const
    {
        return coap::uint_decode<Integer>(data_, size_);
    }

    template <typename Integer>
    void encode(Integer value)
    {
        uint8_t* out = coap::uint_encode(data_, data_ + base_type::max_size_, value);
        size_ = out - data_;
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

    /// Lexical comparison
    template <class Byte2>
    constexpr int compare(const uint<Byte2>& compare_to) const
    {
        return be_uintcmp(
            data_, size_,
            compare_to.data(), compare_to.size());
    }

#if __cpp_impl_three_way_comparison
    // UNTESTED
    template <class Byte2>
    constexpr std::strong_ordering operator<=>(const uint<Byte2>& compare_to) const
    {
        return compare(compare_to) <=> 0;
    }
#else
    template <class Byte2>
    constexpr bool operator==(const uint<Byte2>& compare_to) const
    {
        return compare(compare_to) == 0;
    }
#endif
};

}

using uint = internal::uint<uint8_t>;
using const_uint = internal::uint<const uint8_t>;

}
