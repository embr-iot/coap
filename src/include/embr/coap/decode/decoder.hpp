#pragma once

#include "decoder.h"
#include "../options/decode/stateful.hpp"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
void decoder<Streambuf>::read_header()
{
    static_assert(policy == Presumptive);
    assert(state_ == Header);

    int read = in_.sgetn((pointer)&header_, sizeof(header_));
    good_ = read == sizeof(coap::header);
    good_ &= header_.invariant();

    if(good_) state_ = Token;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::operator>>(token& v) -> decoder&
{
    static_assert(policy == Presumptive);
    assert(state_ == Token);

    const unsigned tkl = header_.tkl();

    // permit a 0-token just for ease of consumption
    v.size = tkl;

    if(tkl == 0)    return *this;

    int read = in_.sgetn((pointer)&v, tkl);
    good_ = read == tkl;

    if(good_)
    {
        // Just incase they go >> flavor
        current_number_ = 0;
        state_ = Options;
    }

    return *this;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F>
errc decoder<Streambuf>::options_decode(F&& f)
{
    if(state_ == Token) state_ = Options;

    assert(state_ == Options);

    bool has_payload{};
    options::decoder<Streambuf&, traits> options(in_);

    errc err = options.dispatch(std::forward<F>(f), &has_payload);

    good_ = err == errc{};

    state_ = has_payload ? Payload : Done;
    return err;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::operator>>(options::option<>& v) -> decoder&
{
    if(state_ == Token)
    {
        current_number_ = 0;
        state_ = Options;
    }
    else
        assert(state_ == Options);

    options::decoder<Streambuf&, traits> options(in_);

    errc err = options.decode_one(&v, &current_number_);

    switch(err)
    {
        case errc{}:  // NOLINT
        {
            state_ = in_.sgetc() == 0xFF ? Payload : Done;
            break;
        }

        // 'alternate' means we unexpectedly hit payload, which we already should have
        // detected in errc{} above.
        // 'bad' is just how it sounds
        default:
            good_ = false;
            break;
    }

    return *this;
}

}
