#pragma once

#include "../../uint/decode.h"
#include "../../uint/encode.h"

#include <estd/cstdint.h>

#include "assert.h"

namespace embr::coap::options {

class block_trailing_byte
{
    uint8_t v_;

    struct mask
    {
        static constexpr unsigned m_pos =   3;  // 0-based
        static constexpr uint8_t m =        1 << m_pos;
        static constexpr uint8_t szx =      0x07;
        static constexpr unsigned num_pos = 4;
        static constexpr unsigned num = 0xF0;
    };

public:
    block_trailing_byte() = default;
    constexpr block_trailing_byte(const block_trailing_byte&) = default;
    constexpr explicit block_trailing_byte(uint8_t raw) : v_{raw}   {};

    void reset()
    {
        v_ = 0;
    }

    constexpr bool more() const
    {
        return (v_ & mask::m) != 0;
    }

    ESTD_CPP_CONSTEXPR(14) void more(bool v)
    {
        v_ &= ~mask::m;
        v_ |= v << mask::m_pos;
    }

    constexpr unsigned szx() const
    {
        return v_ & mask::szx;
    }

    ESTD_CPP_CONSTEXPR(14) void szx(unsigned v)
    {
        v_ &= ~mask::szx;
        v_ |= v;
    }

    constexpr unsigned num() const
    {
        return v_ >> mask::num_pos;
    }

    ESTD_CPP_CONSTEXPR(14) void num(unsigned v)
    {
        v_ &= ~mask::num;
        v_ |= v << mask::num_pos;
    }

    constexpr operator uint8_t() const { return v_; }
};

// Not wire format, although occasionally it overlaps with it.
// Use encode/decode methods for that
class block_value
{
    uint16_t num_;
    block_trailing_byte btb_;

    ESTD_CPP_CONSTEXPR(14) void reset()
    {
        num_ = 0;
        btb_ = block_trailing_byte(0);
    }

public:
    block_value() = default;
    constexpr block_value(const block_value&) = default;

    // DEBT: encode/decode probably ought to freestand

    // 'data' MUST be writable up to 3 bytes
    unsigned encode(uint8_t* data)
    {
        data = uint_encode(data, data + 3, num_);
        *data = btb_;

        return 0;
    }

    void decode(const uint8_t* data, unsigned len)
    {
        if(len == 0)
        {
            reset();
            return;
        }

        --len;

        num_ = uint_decode(data, len);
        btb_ = block_trailing_byte(data[len]);
    }

    constexpr bool more() const
    {
        return btb_.more();
    }

    constexpr unsigned num() const
    {
        return btb_.num() | num_ << 4;
    }

    ESTD_CPP_CONSTEXPR(14) void num(unsigned v)
    {
        num_ = v >> 4;
        btb_.num(v & 0xF);
    }

    // "raw" block size (no precalculation)
    ESTD_CPP_CONSTEXPR(14) void szx(unsigned v)
    {
        assert(v < 8);

        btb_.szx(v);
    }

    // "cooked" block size (precalculated) - remember this pertains not to
    // the "body", just the current block.
    ESTD_CPP_CONSTEXPR(14) unsigned size() const
    {
        const unsigned v = btb_.szx() + 4;
        return v * v;
    }
};

}
