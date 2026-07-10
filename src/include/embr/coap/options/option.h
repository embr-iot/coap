#pragma once

#include "fwd.h"

namespace embr::coap::options {

template <numbers n>
struct option
{
    static constexpr numbers number = n;

    unsigned length;
    union
    {
        const uint8_t* opaque;
        unsigned uint;
        const char* string;
    };
};


}
