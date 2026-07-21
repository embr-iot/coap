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
    good_ &= header_.valid();

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
    if(tkl > 0)
    {
        int read = in_.sgetn((pointer)&v, tkl);
        good_ = read == tkl;
    }

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
    options::decoder<Streambuf&> options(in_);

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

    // Using stateful decoder is a neat trick here but I think using regular options::decoder
    // with some fine tuning (only decode one option) may be preferable
    options::stateful_decoder osd(current_number_);

    estd::errc err = osd.decode(in_, [&](options::option<> op)
        {
            v = op;
        });

    // NOTE: Not quite ready yet, osd.decode looping isn't sensible for movement through Header AND Value
    // and probably won't be until reworking return type

    switch(err)
    {
        case estd::errc{}:  // NOLINT
            state_ = Payload;
            break;

        case estd::errc::resource_unavailable_try_again:
            state_ = Done;
            break;

        default:
            good_ = false;
            break;
    }

    return *this;
}

}
