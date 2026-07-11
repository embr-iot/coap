#pragma once

#include <estd/cstdint.h>
#include <estd/iosfwd.h>

#include "assert.h"

namespace embr::coap::internal {

// Dead-simple output cache.  Should we consider an array out streambuf?  That is probably overkill.
// There's a complicated wrapped/cache ostreambuf if we really want to go that route.
template <unsigned N>
class out_accumulator
{
    // No default initialization so we can live comfortably inside a union

    uint8_t pos_, size_;

public:
    uint8_t buf_[N];

    constexpr int remaining() const { return size_ - pos_; }
    constexpr int size() const { return size_; }

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

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool sputn(Streambuf& out)
    {
        using char_type = typename Streambuf::char_type;

        int written = out.sputn((const char_type*)buf_ + pos_, remaining());
        // DEBT: IIRC written can return -1 on error here, account for that
        pos_ += written;
        assert(pos_ <= size_);
        return pos_ == size_;
    }
};


}
