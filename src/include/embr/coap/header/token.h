#pragma once

#include <estd/cstdint.h>

namespace embr::coap {

struct token
{
    uint8_t value[8];

    // Size of 0 means auto-deduce from header
    uint8_t size{};
};

}