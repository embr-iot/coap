#pragma once

#include <estd/cstdint.h>

#include "assert.h"

namespace embr::coap::options {

#define EMBR_COAP_OPTION_BLOCK_STRICT

class block_trailing_byte
{
    uint8_t v_;

    struct mask
    {
        static constexpr unsigned m_pos =   3;  // 0-based
        static constexpr uint8_t m =        1 << m_pos;
        static constexpr uint8_t szx =      0x07;
    };

public:
    constexpr bool more() const
    {
        return (v_ & mask::m) != 0;
    }

    ESTD_CPP_CONSTEXPR(14) void more(bool v)
    {
        v_ &= ~mask::m;
        v_ |= v << mask::m_pos;
    }

    ESTD_CPP_CONSTEXPR(14) void szx(unsigned v)
    {
        v_ &= ~mask::szx;
        v_ |= v;
    }
};

}
