#include "assert.h"

#include "embr/coap/options/decode.h"

namespace embr::coap {


namespace options {

const uint8_t* delta_length_decode(const uint8_t* in, unsigned number_current, numbers* number, unsigned* length)
{
    using modes = internal::option_enum_base::extended_modes;

    unsigned delta = *in >> 4;
    unsigned length_raw = *in & 0x0F;

    ++in;

    switch(delta)
    {
        case modes::Extended8Bit:
        {
            delta = 13 + *in;
            *number = static_cast<numbers>(number_current + delta);
            break;
        }

        case modes::Extended16Bit:
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
            *length = 13 + *in;
            break;

        case modes::Extended16Bit:
            break;

        case modes::Reserved:
            return nullptr;

        default:
            *length = length_raw;
            break;
    }

    return in;
}

}

}
