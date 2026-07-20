#include "assert.h" // NOLINT

#include "embr/coap/internal/constants.h"
#include "embr/coap/options/decode.h"
#include "embr/coap/options/decode/delta-length-decoder.h"
#include "embr/coap/uint.h"

namespace embr::coap {


namespace options {

//namespace internal {

inline void delta_length_decode_number(const uint8_t* in)
{

}

//}

const uint8_t* delta_length_decode(const uint8_t* in, unsigned number_current, numbers* number, unsigned* length)
{
    using modes = internal::option_enum_base::extended_modes;
    using namespace constants;

    unsigned delta = *in >> 4;
    unsigned length_raw = *in & 0x0F;
    const uint8_t* end = in + *length;

    ++in;

    switch(delta)
    {
        case modes::Extended8Bit:
        {
            delta = option_8_bit_offset + *in;
            *number = static_cast<numbers>(number_current + delta);

            ++in;
            if(in > end)   return nullptr;

            break;
        }

        case modes::Extended16Bit:

            *number = static_cast<numbers>(
                number_current + option_16_bit_offset + uint_decode(in, 2));

            in += 2;
            if(in > end)   return nullptr;

            break;

        case modes::Reserved:
            return nullptr;

        default:
            *number = static_cast<numbers>(number_current + delta);
            break;
    }

    switch(length_raw)
    {
        case modes::Extended8Bit:
            *length = option_8_bit_offset + *in;
            ++in;
            if(in > end)   return nullptr;
            break;

        case modes::Extended16Bit:
            *length = option_16_bit_offset + uint_decode(in, 2);
            in += 2;
            return in > end ? nullptr : in;

        case modes::Reserved:
            return nullptr;

        default:
            *length = length_raw;
            break;
    }

    return in;
}

auto delta_length_decoder::decode_length() -> codes
{
    using namespace constants;

    if(length_ == modes::Extended8Bit)
    {
        length_ = option_8_bit_offset;
        state_ = Length1;
        return More;
    }
    if(length_ == modes::Extended16Bit)
    {
        length_ = option_16_bit_offset;
        state_ = Length2;
        return More;
    }
    if(length_ == modes::Reserved)
        return Bad;

    return Done;
}

auto delta_length_decoder::decode_byte(uint8_t c) -> codes
{
    using namespace constants;
    using modes = internal::option_enum_base::extended_modes;

    switch(state_)
    {
        case Header:
            delta_ = c >> 4;
            length_ = c & 0x0F;

            if(delta_ == modes::Extended8Bit)
            {
                delta_ = option_8_bit_offset;
                state_ = Delta1;
            }
            else if(delta_ == modes::Extended16Bit)
            {
                delta_ = option_16_bit_offset;
                state_ = Delta2;
            }
            else if(delta_ == modes::Reserved)
                return Bad;
            else
            {
                return decode_length();
            }

            break;

        case Delta1:
            delta_ += c;
            return decode_length();

        case Delta2:
            delta_ += c << 8;
            state_ = Delta1;
            break;

        case Length1:
            length_ += c;
            return Done;

        case Length2:
            length_ += c << 8;
            state_ = Length1;
            break;
    }

    return More;
}

void delta_length_decoder::reset()
{
    state_ = Header;
}


}

}
