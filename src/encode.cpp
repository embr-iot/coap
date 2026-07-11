#include "assert.h"

#include "embr/coap/uint.h"
#include "embr/coap/options/encode.h"

namespace embr::coap {


namespace options {

template <bool delta>
static uint8_t* delta_length_encode(uint8_t* const first, uint8_t* out, unsigned num)
{
    using modes = internal::option_enum_base::extended_modes;

    auto apply_first = [first](unsigned v)
    {
        if constexpr(delta)
            *first = v << 4;
        else
            *first |= v;
    };

    if(num < 13)
    {
        apply_first(num);
        return out;
    }
    else if(num < 269)
    {
        apply_first(modes::Extended8Bit);
        *out++ = num - 13;
        return out;
    }

    apply_first(modes::Extended16Bit);
    return uint_encode(out, num - 269, 2);
}

uint8_t* delta_length_encode(uint8_t* out, unsigned current, numbers number, unsigned length)
{
    uint8_t* const first = out++;

    assert(number >= current);

    const unsigned option_delta = number - current;

    out = delta_length_encode<true>(first, out, option_delta);
    return delta_length_encode<false>(first, out, length);
}

void stateful_encoder::number_and_length(numbers n, unsigned length)
{
    const uint8_t* end = delta_length_encode(temp_.buf_, current_number_, n, length);
    current_number_ = n;
    temp_.size_ = end - temp_.buf_;
}

}

}