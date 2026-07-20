#pragma once

#include "decoder.h"

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

}
