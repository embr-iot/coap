#include "assert.h" // NOLINT

#include "embr/coap/internal/constants.h"
#include "embr/coap/options/encode.h"
#include "embr/coap/uint.h"

namespace embr::coap {


namespace options {

/// Low level options number delta/length encoder
/// @param first always first byte, indicating core delta/length info
/// @param out current position for further encoding.  always first + 1 for delta, but can vary after that
/// @param num either number_delta or length
/// @return
///
template <bool delta>
static uint8_t* delta_length_encode(uint8_t* const first, uint8_t* out, unsigned num)
{
    using namespace constants;
    using modes = internal::option_enum_base::extended_modes;

    // CoAP option length can be just a little bit bigger than 16-bit maximum, though so
    // far the biggest option I've seen is under 256 bytes
    assert(num < 0xFFFF + option_16_bit_offset);
    assert(!delta || out == first + 1);

    auto apply_first = [first](unsigned v)
    {
        if constexpr(delta)
            *first = v << 4U;
        else
            *first |= v;
    };

    if(num < option_8_bit_offset)
    {
        apply_first(num);
        return out;
    }
    if(num < option_16_bit_offset)
    {
        apply_first(modes::Extended8Bit);
        *out = num - option_8_bit_offset;
        return out + 1;
    }

    apply_first(modes::Extended16Bit);
    uint_encode_fixed(out, out + 2, num - option_16_bit_offset);
    return out + 2;
}

uint8_t* delta_length_encode(uint8_t* out, unsigned current, numbers number, unsigned length)
{
    uint8_t* const first = out++;

    assert(number >= current);

    const unsigned option_delta = number - current;

    out = delta_length_encode<true>(first, out, option_delta);
    return delta_length_encode<false>(first, out, length);
}

void stateful_encoder::encode(numbers n, unsigned length)
{
    const uint8_t* end = delta_length_encode(temp_.buf_, current_number_, n, length);
    current_number_ = n;
    temp_.init(end - temp_.buf_);
}

}   // namespace options

}   // namespace embr::coap
