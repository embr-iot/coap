#pragma once

#include <estd/algorithm.h>
#include <estd/cstdint.h>
#include <estd/initializer_list.h>

namespace embr::coap {

#pragma pack(push, 1)

// DEBT: Use estd::layer1::vector

struct token
{
    static constexpr unsigned max_size = 8;

    uint8_t value[max_size];

    // Size of 0 means auto-deduce from header
    uint8_t size;

    token() = default;

    // 06OCT16 MB DEBT: keep an eye on std::initializer_list
    // here.  Also, https://github.com/malachi-iot/estdlib/issues/241
    // keeps us from doing a constexpr ESTD_CPP_CONSTEXPR(14)
    ESTD_CPP_CONSTEXPR(20) token(std::initializer_list<uint8_t> values, uint8_t size = 0) :
        value{},
        size{size}
    {
        estd::copy(values.begin(), values.end(), value);
    }
};

#pragma pack(pop)

}