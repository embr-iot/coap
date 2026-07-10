#pragma once

#include "fwd.h"
#include "option.h"
#include "markers.h"
#include "traits.h"

#include "../uint.h"

#include <estd/cstdint.h>
#include <estd/istream.h>

namespace embr::coap::options {

// One-shot flavor
const uint8_t* delta_length_decode(const uint8_t* in, unsigned number_current, numbers* number, unsigned* length);

// State machine flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
class delta_length_decoder
{
    enum states : uint8_t
    {
        Header,
        Delta1,
        Delta2,
        Length1,
        Length2,
    };

    states state_{Header};

    // Technically CoAP supports delta/length > 65536 - but that isn't happening
    uint16_t delta_{};
    uint16_t length_{};

public:
    enum codes
    {
        Done,
        More,
        Bad
    };

    codes decode_length();  // DEBT: Make internal
    codes decode_byte(uint8_t c);

    using modes = internal::option_enum_base::extended_modes;

    constexpr unsigned delta() const { return delta_; }
    constexpr unsigned length() const { return length_; }
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
    enum policies
    {
        // Presumptive means we presume all input data is available all the time
        Presumptive,
        // Retry means if the data we need isn't ready yet, keep retrying until it
        // is (effectively poll-blocking).  Note that there are two possibilities for
        // retry:
        // 1. LwIP pbuf/netbuf style where data IS immediately available but requires a boundary hop
        // 2. Typical slower-than-instant appearance of data
        // We may want two flavors of Retry policy to reflect this
        Retry
    };

    using errc = estd::errc;

    Streambuf in_;
    unsigned current_number_;

    template <numbers, class F>
    estd::errc dispatch_ll(F&&, unsigned len);

    template <class F>
    estd::errc dispatch(F&&, numbers number, unsigned len);

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    template <class F>
    estd::errc decode(F&&);
};

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <numbers number, class F>
estd::errc decoder<Streambuf>::dispatch_ll(F&& f, unsigned len)
{
    constexpr policies policy = Presumptive;

    using traits = option_traits<number>;
    option<number> o;

    o.length = len;

    unsigned avail = in_.in_avail();
    uint8_t temp[32];

    if(avail < len)
    {
        if constexpr(policy == Presumptive)
        {
            // Input stream is presumed to be incorrectly truncated in this mode.
            // Since it is akin to user input, this is not an assert but a validation class
            // error
            // NOTE: I would have used 'interrupted' but estd's fallback alias doesn't have that guy yet
            return errc::no_message_available;
        }
        // Do blocking/repeat read here if policy permits it
        //in_.sgetn(temp, len);
    }

    auto data = (const uint8_t*)in_.gptr();

    if constexpr(traits::format == value_formats::Opaque)
    {
        o.opaque_ = data;
    }
    else if constexpr(traits::format == value_formats::String)
    {
        o.string_ = (const char*)data;
    }
    else if constexpr(traits::format == value_formats::Uint)
    {
        o.uint_ = uint_decode<unsigned>(data, len);
    }

    f(o);

    in_.pubseekoff(len, estd::ios_base::cur);

    return {};
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F>
estd::errc decoder<Streambuf>::dispatch(F&& f, numbers number, unsigned len)
{
    using n = numbers;

    switch(number)
    {
        case n::UriHost:
            return dispatch_ll<n::UriHost>(std::forward<F>(f), len);

        case n::UriPath:
            return dispatch_ll<n::UriPath>(std::forward<F>(f), len);

        case n::ContentFormat:
            return dispatch_ll<n::ContentFormat>(std::forward<F>(f), len);

        default:
            break;
    }

    return {};
}


template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F>
estd::errc decoder<Streambuf>::decode(F&& f)
{
    delta_length_decoder dlc;

    using r = delta_length_decoder::codes;

    int c2;

    // EXPERIMENTAL - too side-effecty?  Or cool?
    auto inc = [&]
    {
        c2 = in_.sbumpc();
        // DEBT: EOS not same as EOL or maybe EOF
        return c2 != 0xFF && c2 != -1;
    };

    for(;;)
    {
        int c = in_.sbumpc();

        // Payload or end of stream
        if(c == 0xFF || c == -1)
        {
            return {};
        }

        r ret = dlc.decode_byte(c);

        if(ret == r::Done)
        {
            current_number_ += dlc.delta();
            errc err = dispatch(std::forward<F>(f), (numbers)current_number_, dlc.length());
            if(err != errc{}) return err;
        }
        else if(ret == r::Bad)
        {
            return errc::no_message_available;
        }
        else
            assert(ret == r::More);
    }
}


}
