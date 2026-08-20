#pragma once

#include "header/codes.h"
#include "fwd.h"

#include <estd/bit.h>
#include <estd/cstdint.h>
#include <estd/optional.h>

namespace embr::coap {

// NOLINTBEGIN(*-magic-numbers)

#pragma pack(push, 1)

#if __GNUC__
#define FEATURE_EMBR_TYPE_PUNNING 1
#endif

class header : public internal::header_base
{
    static constexpr unsigned VER_MASK = 0b11000000;
    static constexpr unsigned TYPE_MASK = 0b00110000;
    static constexpr unsigned TKL_MASK = 0b00001111;

    // Interesting!  Even though GCC does type punning, it still kicks back in a constexpr, so not unionizing these
    //uint8_t raw_array[4];
    //uint32_t raw_int;

    uint8_t ver_t_tkl_;
    codes code_;
    uint16_t mid_;

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

    constexpr explicit header(const uint8_t* raw) :
        //raw_array{raw[0], raw[1], raw[2], raw[3]}
        ver_t_tkl_{raw[0]},
        code_{raw[1]},
        // Remember mid_ is stored in its native byte order, despite being uint16_t
        mid_{static_cast<uint16_t>(raw[2] << 8 | raw[3])}
    {

    }

public:
    header() = default;
    constexpr explicit header(estd::nullopt_t) :
        ver_t_tkl_{0x40},
        code_{},
        mid_{}
    {}

    constexpr explicit header(types type, codes code, unsigned tkl, uint16_t mid) :
        ver_t_tkl_{ver_t_tkl(type, tkl)},
        code_{code},
        mid_{swap(mid)}
    {}

    constexpr explicit header(types type, codes code, uint16_t mid = 0) :
        ver_t_tkl_{ver_t_tkl(type, 0)},
        code_{code},
        mid_{swap(mid)}
    {}

    [[nodiscard]] constexpr uint16_t mid() const { return swap(mid_); }

    constexpr void mid(uint16_t v) { mid_ = swap(v); }

    constexpr void tkl(unsigned v)
    {
        assert(v <= 8);

        ver_t_tkl_ = (ver_t_tkl_ & ~TKL_MASK) | v;
    }

    [[nodiscard]] constexpr unsigned tkl() const
    {
        return ver_t_tkl_ & TKL_MASK;
    }

    [[nodiscard]] constexpr unsigned ver() const
    {
        return ver_t_tkl_ >> 6;
    }

    constexpr void type(types v)
    {
        ver_t_tkl_ = (ver_t_tkl_ & ~TYPE_MASK) | (v << 4);
    }

    [[nodiscard]] constexpr types type() const
    {
        return static_cast<types>((ver_t_tkl_ & TYPE_MASK) >> 4);
    }

    constexpr void code(codes r) { code_ = r; }

    [[nodiscard]] constexpr codes code() const { return code_; }

    constexpr bool operator==(const header& compare_to) const
    {
        // NOTE: Not doing a raw union 32 bit compare here to avoid presuming type punning.
        return ver_t_tkl_ == compare_to.ver_t_tkl_ &&
            code_ == compare_to.code_ &&
            mid_ == compare_to.mid_;
    }

    [[nodiscard]] constexpr bool invariant() const
    {
        return ver() == 1 && code_ > 0 && tkl() <= 8;
    }

    static constexpr header from(const uint8_t raw[4])
    {
        return header(raw);
    }

    static header from(uint32_t v)
    {
        return header(reinterpret_cast<uint8_t*>(&v));
    }
};

#pragma pack(pop)

static_assert(sizeof(header) == 4);

// NOLINTEND(*-magic-numbers)

}
