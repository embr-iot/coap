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
    uint16_t delta_;
    uint16_t length_;

public:
    enum codes
    {
        Done,
        More,
        Bad
    };

    codes decode_length();  // DEBT: Make internal
    codes decode_byte(uint8_t c);

    void reset();

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
        // Non-contiguous means we operate similarly to Retry.  However, in this mode
        // we still presume all data is immediately available - merely that it takes
        // multiple reads to get it.  This reflects LwIP pbuf/netbuf style where data
        // IS immediately available but requires a boundary hop
        NonContiguous,
        // Retry means if the data we need isn't ready yet, keep retrying until it
        // is (effectively poll-blocking).
        Retry
    };

    using errc = estd::errc;

    Streambuf in_;
    unsigned current_number_;

    template <numbers, class F, class Retry>
    estd::errc dispatch_ll(F&&, unsigned len, Retry&&);

    template <class F, class Retry = estd::monostate>
    estd::errc dispatch(F&&, numbers number, unsigned len, Retry&& = {});

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    template <class F>
    estd::errc decode(F&&);
};

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <numbers number, class F, class Retry2>
estd::errc decoder<Streambuf>::dispatch_ll(F&& f, unsigned len, Retry2&& retry)
{
    constexpr policies policy = Presumptive;
    constexpr unsigned temp_sz = policy == Presumptive ? 0 : 64;

    using traits = option_traits<number>;
    option<number> o;

    o.length = len;

    unsigned avail = in_.in_avail();
    uint8_t temp[temp_sz];

    if(avail < len)
    {
        // DEBT: temp buffer may sometimes may not be big enough, since options technically can get pretty big.
        // account for this with a validation class error (don't want poison packets bringing us down)
        if constexpr(policy == Presumptive)
        {
            // Input stream is presumed to be incorrectly truncated in this mode.
            // Since it is akin to user input, this is not an assert but a validation class
            // error
            // NOTE: I would have used 'interrupted' but estd's fallback alias doesn't have that guy yet
            return errc::no_message_available;
        }
        else if constexpr(policy == Retry)
        {
            // Do blocking/repeat read here
            //in_.sgetn(temp, len);
        }
        else
            static_assert(false, "Unsupported policy");
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

    // invalid_argument can be considered a warning, not an error
    if(len < traits::min_length)
    {
        return errc::invalid_argument;
    }
    else if(traits::max_length != 0 && len > traits::max_length)
    {
        return errc::invalid_argument;
    }

    return {};
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F, class Retry2>
estd::errc decoder<Streambuf>::dispatch(F&& f, numbers number, unsigned len, Retry2&& retry)
{
    using n = numbers;

    switch(number)
    {
        case n::Accept:
            return dispatch_ll<n::Accept>(std::forward<F>(f), len, std::forward<Retry2>(retry));

        case n::ContentFormat:
            return dispatch_ll<n::ContentFormat>(std::forward<F>(f), len, std::forward<Retry2>(retry));

        case n::UriHost:
            return dispatch_ll<n::UriHost>(std::forward<F>(f), len, std::forward<Retry2>(retry));

        case n::UriPath:
            return dispatch_ll<n::UriPath>(std::forward<F>(f), len, std::forward<Retry2>(retry));

        case n::UriPort:
            return dispatch_ll<n::UriPort>(std::forward<F>(f), len, std::forward<Retry2>(retry));

        case n::UriQuery:
            return dispatch_ll<n::UriQuery>(std::forward<F>(f), len, std::forward<Retry2>(retry));

        default:
        {
            option2 o;

            o.number = number;
            o.length = len;
            o.opaque_ = nullptr;    // FIX: Actually assign this guy, presume opaque is what is wanted for unknowns

            in_.pubseekoff(len, estd::ios_base::cur);

            break;
        }
    }

    return {};
}


template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F>
estd::errc decoder<Streambuf>::decode(F&& f)
{
    delta_length_decoder dlc;

    using r = delta_length_decoder::codes;

    auto valid = [](int c)
    {
        // DEBT: EOS not same as EOL or maybe EOF
        return c != 0xFF && c != -1;
    };

    // EXPERIMENTAL - too side-effecty?  Or cool?
    auto bump = [&](int& c)
    {
        c = in_.sbumpc();
        return valid(c);
    };

    for(int c; bump(c); )
    {
        r ret = dlc.decode_byte(c);

        if(ret == r::Done)
        {
            current_number_ += dlc.delta();
            errc err = dispatch(std::forward<F>(f), (numbers)current_number_, dlc.length());
            if(err != errc{} && err != errc::invalid_argument) return err;

            dlc.reset();
        }
        else if(ret == r::Bad)
        {
            return errc::no_message_available;
        }
        else
            assert(ret == r::More);
    }

    return {};
}

// IDEA CAPTURE
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_with_retry();

// IDEA CAPTURE
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_noncontiguous();

// Almost works, still experimental - in c++17 too opportunistically eats up
// things that you'd hope estd::span would resolve
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_exp(Streambuf& in, F&& f)
{
    decoder<Streambuf&> d(in);

    return d.decode(std::forward<F>(f));
}

// UNTESTED
template <class T, class F>
estd::errc decode(estd::span<T> in, F&& f)
{
    using streambuf_type = estd::detail::basic_ispanbuf<T>;

    decoder<streambuf_type> d(in);

    return d.decode(std::forward<F>(f));
}

}
