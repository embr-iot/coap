#pragma once

#include <estd/cstdint.h>
#include <estd/iosfwd.h>

#include "assert.h"

namespace embr::coap::internal {

// Dead-simple output cache.  Should we consider an array out streambuf?  That is probably overkill.
// There's a complicated wrapped/cache ostreambuf if we really want to go that route.
template <unsigned N, unsigned width = 8, bool with_ext = false>
class __attribute__((packed)) out_accumulator
{
    // No default initialization so we can live comfortably inside a union

    // with_ext & ext_mask are EXPERIMENTAL
    static constexpr unsigned ext_mask = with_ext ?
        (1 << (width - 1)) : 0;

    unsigned
        pos_ : width,
        size_ : width;

public:
    out_accumulator() = default;

    union
    {
        uint8_t buf_[N];
        const uint8_t* ext_;
    };

    constexpr int pos() const { return pos_; }
    constexpr int remaining() const { return size_ - pos_; }
    constexpr unsigned size() const
    {
        return size_ & ~ext_mask;
    }

    static constexpr unsigned max_size() { return N; }

    constexpr bool is_ext() const
    {
        static_assert(with_ext);

        return size_ >> (width - 1);
    }

    void init(unsigned sz = 0)
    {
        assert(sz <= N);

        pos_ = 0;
        size_ = sz;
    }

    void init(const void* from, unsigned sz)
    {
        init(sz);

        memcpy(buf_, from, sz);
    }

    void init_ext(const void* from, unsigned sz)
    {
        static_assert(with_ext);

        pos_ = 0;
        size_ = sz | ext_mask;

        ext_ = (const uint8_t*) from;
    }

    /// @brief sputn cached output
    /// @param out
    /// @return true if all desired data was written, false otherwise
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool sputn(Streambuf& out)
    {
        using char_type = typename Streambuf::char_type;

        int written = out.sputn((const char_type*)buf_ + pos_, remaining());
        // DEBT: IIRC written can return -1 on error here, account for that
        pos_ += written;
        assert(pos_ <= size());
        return pos_ == size();
    }

    // EXPERIMENTAL
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool sputn_ext(Streambuf& out)
    {
        static_assert(with_ext);

        using char_type = typename Streambuf::char_type;

        int written = out.sputn((const char_type*)ext_ + pos_, remaining());
        // DEBT: IIRC written can return -1 on error here, account for that
        pos_ += written;
        assert(pos_ <= size());
        return pos_ == size();
    }
};

static_assert(sizeof(out_accumulator<8>) == 10);

static_assert(sizeof(out_accumulator<8, 4>) == 9);
static_assert(sizeof(out_accumulator<8, 16>) == 12);


}
