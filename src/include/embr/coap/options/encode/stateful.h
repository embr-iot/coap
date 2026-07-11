#pragma once

#include "fwd.h"

#include <estd/cstdint.h>

#include "assert.h"

namespace embr::coap::options {

// Dead-simple output cache.  Should we consider an array out streambuf?  That is probably overkill.
// There's a complicated wrapped/cache ostreambuf if we really want to go that route.
template <unsigned N>
struct out_accumulator
{
    uint8_t pos_{};
    uint8_t size_{};
    uint8_t buf_[N];

    void init(const void* from, unsigned sz)
    {
        assert(sz < N);

        memcpy(buf_, from, sz);
        pos_ = 0;
        size_ = sz;
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool sputn(Streambuf& out)
    {
        using char_type = typename Streambuf::char_type;

        int remaining = size_ - pos_;
        int written = out.sputn((const char_type*)buf_ + pos_, remaining);
        pos_ += written;
        return written == remaining;
    }
};

class stateful_encoder
{
#if UNIT_TESTING
public:
#endif
    uint16_t current_number_{};
    out_accumulator<5> temp_;

    void number_and_length(numbers n, unsigned length);

public:
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_length(Streambuf& out)  { return temp_.sputn(out); }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_length(Streambuf& out, numbers n, unsigned length)
    {
        number_and_length(n, length);
        return temp_.sputn(out);
    }
};

}