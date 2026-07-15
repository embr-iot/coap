#pragma once

#include "header/codes.h"
#include "fwd.h"

#include <estd/bit.h>
#include <estd/cstdint.h>
#include <estd/optional.h>

namespace embr::coap {

#pragma pack(push, 1)

// See https://datatracker.ietf.org/doc/html/rfc7252#section-12.1.1
class header : public internal::header_base
{
    static constexpr unsigned VER_MASK = 0b11000000;
    static constexpr unsigned TYPE_MASK = 0b00110000;
    static constexpr unsigned TKL_MASK = 0b00001111;

    uint8_t ver_t_tkl_;
    uint8_t code_;
    uint16_t mid_;

public:
    enum types
    {
        CON,
        NON,
        ACK,
        RST
    };

private:
    static constexpr uint8_t ver_t_tkl(types type, unsigned tkl)
    {
        return 0x40 | (type << 4) | tkl;
    }

    static constexpr uint16_t swap(uint16_t v)
    {
        if constexpr(estd::endian::native == estd::endian::little)
            return estd::byteswap(v);
        else
            return v;
    }

public:
    header() = default;
    constexpr explicit header(estd::nullopt_t) :
        ver_t_tkl_{0x40},
        code_{},
        mid_{}
    {}

    constexpr explicit header(types type, uint8_t code, unsigned tkl, uint16_t mid) :
        ver_t_tkl_{ver_t_tkl(type, tkl)},
        code_{code},
        mid_{swap(mid)}
    {}

    constexpr explicit header(types type, uint8_t code, uint16_t mid = 0) :
        ver_t_tkl_{ver_t_tkl(type, 0)},
        code_{code},
        mid_{swap(mid)}
    {}

    constexpr uint16_t mid() const { return swap(mid_); }

    constexpr void mid(uint16_t v) { mid_ = swap(v); }

    void tkl(unsigned v)
    {
        assert(v <= 8);

        ver_t_tkl_ = (ver_t_tkl_ & ~TKL_MASK) | v;
    }

    constexpr unsigned tkl() const
    {
        return ver_t_tkl_ & TKL_MASK;
    }

    constexpr unsigned ver() const
    {
        return ver_t_tkl_ >> 6;
    }

    constexpr void type(types v)
    {
        ver_t_tkl_ = (ver_t_tkl_ & ~TYPE_MASK) | (v << 4);
    }

    constexpr types type() const
    {
        return static_cast<types>((ver_t_tkl_ & TYPE_MASK) >> 4);
    }

    constexpr void code(request_codes r) { code_ = r; }

    constexpr void code(response_codes r) { code_ = r; }

    constexpr request_codes request_code() const { return static_cast<request_codes>(code_); }

    constexpr bool operator==(const header& compare_to) const
    {
        // NOTE: Not doing a raw union 32 bit compare here to avoid presuming type punning.
        return ver_t_tkl_ == compare_to.ver_t_tkl_ &&
            code_ == compare_to.code_ &&
            mid_ == compare_to.mid_;
    }

    constexpr bool valid() const
    {
        return ver() == 1 && code_ > 0;
    }
};

#pragma pack(pop)

static_assert(sizeof(header) == 4);

const char* to_string(header::types);

}
