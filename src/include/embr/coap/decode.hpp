#pragma once

#include "decode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
estd::errc stateful_decoder::sgetn(Streambuf& in, uint8_t* data, unsigned sz)
{
    int remaining = sz - pos_;
    int read = in.sgetn(data + pos_, remaining);

    pos_ += read;

    return read == remaining ? errc{} : errc::operation_in_progress;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
estd::errc stateful_decoder::poll_one(Streambuf& in, header* h)
{
    assert(state_ == Header);
    return {};
}

template <auto value>
using auto_constant = estd::integral_constant<decltype(value), value>;

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc stateful_decoder::poll_one(Streambuf& in, F&& f)
{
    estd::errc err;

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

            if(err == errc::operation_in_progress)
                // DEBT: Don't do recursion
                poll_one(in, f);

            break;

    }

    return {};
}

}
