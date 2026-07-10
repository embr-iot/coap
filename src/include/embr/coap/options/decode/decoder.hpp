#pragma once

#include "decoder.h"
#include "../numbers.h"
#include "delta-length-decoder.h"

#include "../traits.h"
#include "../option.h"
#include "../../uint.h"

namespace embr::coap::options {

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
    unsigned current_number = 0;

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
            current_number += dlc.delta();
            errc err = dispatch(std::forward<F>(f), (numbers)current_number, dlc.length());
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


}
