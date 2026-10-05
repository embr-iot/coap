#pragma once

#include <estd/cstdint.h>

namespace embr::coap {

#pragma pack(push, 1)

// DEBT: Use estd::layer1::vector

struct token
{
    static constexpr unsigned max_size = 8;

    uint8_t value[max_size];

    // Size of 0 means auto-deduce from header
    uint8_t size;
};

#pragma pack(pop)

}