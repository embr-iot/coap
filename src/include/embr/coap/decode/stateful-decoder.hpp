#pragma once

#include "stateful-decoder.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
errc stateful_decoder::sgetn(Streambuf& in, uint8_t* data, unsigned sz)
{
    int remaining = sz - pos_;
    int read = in.sgetn(data + pos_, remaining);

    pos_ += read;

    return read == remaining ? errc{} : errc::again;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
errc stateful_decoder::sgetn_exp(Streambuf& in, uint8_t** data, unsigned sz)
{
    // EXPERIMENTAL
    // Probably a non-starter, these paradigms differ a little too much

    if(in.in_avail() >= sz)
    {
        auto data = *(const uint8_t*)in.gptr();
    }
    else
    {
        return sgetn(in, &accumulator_, sz);
    }
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
errc stateful_decoder::poll_one(Streambuf& in, header* h)
{
    assert(state_ == Header);
    errc err = sgetn(in, (uint8_t*)h, 4);

    if(err == errc{})
    {
        pos_ = 0;
        size_ = h->tkl();
        state_ = size_ ? Token : Options;
    }

    return err;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
errc stateful_decoder::poll_one(Streambuf& in, token* t)
{
    if(state_ == Options)
    {
        // We can reach here if:
        // 1. Caller doesn't notice 0 tkl and calls poll_one(token) anyway
        // 2. Caller accidentally calls token twice with tkl > 0 (system error, moved on to Options already)
        assert(size_ == 0);
        return errc{};
    }
    assert(state_ == Token);
    errc err = sgetn(in, t->value, size_);

    if(err == errc{})
    {
        t->size = size_;
        state_ = Options;
    }

    return err;
}

template <auto value>
using auto_constant = estd::integral_constant<decltype(value), value>;

// TBD Expect one of 3 error codes:
// {} = do an immediate poll_one_ll again
// resource_unavailable_try_again = full poll cycle, exit poll_one
// ??? = end of data stream reached
// or with new experimental local errc:
// cycle = do an immediate poll again
// again = full poll cycle, exit poll_one (maybe do sync, delay, etc use case dependent)
// done = end of data stream reached
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
errc stateful_decoder::poll_one_ll(Streambuf& in, F&& f)
{
    errc err;

    switch(state_)
    {
        case Header:
            err = sgetn(in, accumulator_, 4);
            if(err == errc{})
            {
                auto h = (header*)accumulator_;
                f(auto_constant<Header>{}, *h);
                state_ = h->tkl() > 0 ? Token : Options;
                pos_ = 0;   // reset accumulator
                size_ = h->tkl();

                // DEBT: Don't do recursion
                poll_one(in, f);
            }
            break;

        case Token:
            err = sgetn(in, accumulator_, size_);
            if(err == errc{})
            {
                f(auto_constant<Token>{}, estd::span<uint8_t>(accumulator_, size_));
                state_ = Options;

                // DEBT: Don't do recursion
                poll_one(in, f);
            }
            break;

        case Option:
            err = options_.decode_one(in, [&](const options::option<>& o)
            {
                f(auto_constant<Option>{}, o);
            });

            if(err == errc::cycle)
                // DEBT: Don't do recursion
                poll_one(in, f);

            break;

        // TBD: Payload and Done
        default:
            break;

    }

    return {};
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
errc stateful_decoder::poll_one(Streambuf& in, F&& f)
{
    return poll_one_ll(in, f);
    /*
    errc err;

    while((err = poll_one_ll(in, std::forward<F>(f)) == errc{}))
    {

    }   */
}

}
