#pragma once

#include <estd/cstdint.h>

namespace embr::coap {

#pragma pack(push, 1)

// DEBT: Use estd::layer1::vector

struct token
{
    uint8_t value[8];

    // Size of 0 means auto-deduce from header
    uint8_t size{};
};

#pragma pack(pop)

}