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

    if(good_)
    {
        state_ = header_.tkl() > 0 ? Token : Options;
    }
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::operator>>(token& v) -> decoder&
{
    static_assert(policy == Presumptive);
    // permit a 0-token just for ease of consumption
    assert(state_ == Token || state_ == Options);
    unsigned tkl = header_.tkl();
    v.size = tkl;
    if(tkl > 0)
    {
        int read = in_.sgetn((pointer)&v, tkl);
        good_ = read == tkl;
    }

    if(good_)
    {
        state_ = Options;
    }

    return *this;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::operator>>(options::option<>& v) -> decoder&
{
    assert(state_ == Options);

    // DEBT: Really an additional state would probably be better
    if(!current_number_initialized_)
    {
        current_number_ = 0;
        current_number_initialized_ = true;
    }

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
