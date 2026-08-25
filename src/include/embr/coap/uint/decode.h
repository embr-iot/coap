#pragma once

#include <estd/cstdint.h>
#include <estd/cstdlib.h>

#include <embr/internal/word/packer.h>

namespace embr::coap {

namespace internal {

// TODO: Once embr runtime flavor of packer is present, try using this instead
// of explicit uint_decode (and maybe uint_encode).
template <typename Integer>
using packer = embr::internal::packer<Integer, 0, estd::endian::big>;

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

}
