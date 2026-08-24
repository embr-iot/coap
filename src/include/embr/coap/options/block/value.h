#pragma once

#include "../../uint/decode.h"
#include "../../uint/encode.h"

#include <estd/cstdint.h>
#include <estd/cmath.h>

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
    constexpr explicit block_trailing_byte(unsigned num, bool m, unsigned szx) :
        v_{num << mask::num_pos | m << mask::m_pos || szx}
    {}

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
// DEBT: Rework so that num_ is a 'raw word' composed of a big-endian byte array.  In this way,
// encode/decode become a matter of positioning into block_value
class block_value
{
    uint16_t num_;
    block_trailing_byte btb_;

public:
    ESTD_CPP_CONSTEXPR(14) void reset()
    {
        num_ = 0;
        btb_ = block_trailing_byte(0);
    }

    static constexpr bool valid_num(unsigned v)
    {
        return v <= 0xFFFFFF;
    }

    static constexpr bool valid_szx(unsigned v)
    {
        return v < 8;
    }

    block_value() = default;
    constexpr block_value(const block_value&) = default;

    constexpr explicit block_value(unsigned num, bool m, unsigned szx) :
        num_{static_cast<uint16_t>(num >> 4)},
        btb_{num & 0x0F, m, szx}
    {
        // Truth is I can't think of a non-synthetic use case where this constructor is useful as a constexpr.
        // But, I did it anyway
#if __cpp_constexpr >= 201304L
        assert(valid_num(num));
        assert(valid_szx(szx));
#endif
    }

    // DEBT: encode/decode probably ought to freestand

    // 'data' MUST be writable up to 3 bytes
    unsigned encode(uint8_t* data)
    {
        if(num_ == 0 && btb_ == 0)  return 0;

        uint8_t* v = uint_encode(data, data + 2, num_);
        *v = btb_;

        return 1 + v - data;
    }

    bool decode(const uint8_t* data, unsigned len)
    {
        if(len == 0)
        {
            reset();
            return true;
        }

        if(--len > 2)   return false;

        num_ = uint_decode(data, len);
        btb_ = block_trailing_byte(data[len]);

        return true;
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
        assert(valid_num(v));

        num_ = v >> 4;
        btb_.num(v & 0x0F);
    }

    constexpr unsigned szx() const
    {
        return btb_.szx();
    }

    // "raw" block size (no precalculation)
    ESTD_CPP_CONSTEXPR(14) void szx(unsigned v)
    {
        assert(valid_szx(v));

        btb_.szx(v);
    }

    // "cooked" block size (precalculated) - remember this pertains not to
    // the "body", just the current block.
    ESTD_CPP_CONSTEXPR(14) unsigned size() const
    {
        return estd::pow(2, btb_.szx() + 4);
    }
};

}
