#pragma once

#include <estd/cstdint.h>
#include <estd/istream.h>

#include "fwd.h"
#include "markers.h"

namespace embr::coap::options {

// One-shot flavor
const uint8_t* delta_length_decode(const uint8_t* in, unsigned number_current, numbers* number, unsigned* length);

// State machine flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
class delta_length_decoder
{
    enum states
    {
        Header,
        Delta1,
        Delta2,
        Length1,
        Length2
    };

    states state_{Header};

    // Technically CoAP supports delta/length > 65536 - but that isn't happening
    uint16_t delta_{};
    uint16_t length_{};

public:
    void decode_byte(uint8_t c);
};

/*
template <numbers n, ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, bool has_grandparent = false>
class single_decoder
{
    using decoder_type = decoder<Streambuf>;
    using streambuf_type = typename estd::remove_reference_t<Streambuf>;

    decoder_type* parent_;

public:
    constexpr single_decoder(decoder_type* decoder) :
        parent_{decoder}
    {}
};  */

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder
{
    Streambuf in_;
    unsigned current_number_;

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    template <class F>
    void decode(F&&);
};

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F>
void decoder<Streambuf>::decode(F&& f)
{
    uint8_t buf[5];

    int read = in_.sgetn(buf, 5);

    if(read > 0)
    {
    }
}


}
