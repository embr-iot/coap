#pragma once

#include "header.h"
#include "options/decode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder
{
    Streambuf in_;
    using streambuf_type = Streambuf;
    using char_type = typename streambuf_type::char_type;
    using pointer = char_type*;

    using options_decoder_type = options::decoder<Streambuf&>;

    options_decoder_type options_;

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) :
        in_{std::forward<Args>(args)...},
        options_{in_}
    {}

    decoder& operator>>(header& h)
    {
        in_.sgetn((pointer)&h, sizeof(h));
        return *this;
    }

    /* No not gonna work well this way, need a callback flavor
    template <options::numbers n>
    options::single_decoder<n, Streambuf>& operator>>(options::option_marker<n>)
    {
        return { &options_ };
    }   */
};

}
