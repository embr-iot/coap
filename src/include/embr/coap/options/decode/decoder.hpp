#pragma once

#include "decoder.h"
#include "../numbers.h"
#include "delta-length-decoder.h"
#include "delta-length-decode.hpp"

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
template <class F, class NoMatchFunctor, class Retry2>
estd::errc decoder<Streambuf>::dispatch(F&& f, NoMatchFunctor&& no_match, numbers number, unsigned len, Retry2&& retry)
{
    errc err = dispatch_number_ll(number,
        [&](auto number)
        {
            return dispatch_ll<number>(std::forward<F>(f), len, std::forward<Retry2>(retry));
        },
        [&](numbers)
        {
            // no_match path is optional, oftentimes we don't care much
            if constexpr(!estd::is_same_v<NoMatchFunctor, estd::monostate>)
            {
                option o;

                o.number = number;
                o.length = len;
                o.opaque_ = nullptr;    // FIX: Actually assign this guy, presume opaque is what is wanted for unknowns

                f(o);
                return errc{};
            }

            // In the case where we are ignoring unmatched arguments, indicate
            // a warning state
            return errc::invalid_argument;
        });

    // DEBT: Make sure this proceeds forward correctly
    in_.pubseekoff(len, estd::ios_base::cur);

    return err;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F, class NoMatchFunctor>
estd::errc decoder<Streambuf>::decode(F&& f, bool* has_payload, NoMatchFunctor&& no_match)
{
    unsigned current_number = 0;

    auto f2 = [&](const delta_length_decoder& dld)
    {
        current_number += dld.delta();
        errc err = dispatch(
            std::forward<F>(f),
            std::forward<NoMatchFunctor>(no_match),
            (numbers)current_number, dld.length());
        // FIX: Heed return code
        //if(err != errc{} && err != errc::invalid_argument) return err;
    };

    estd::optional<int> c;

    while(!(c = delta_length_decode(in_, f2)).has_value())
    {
    }

    *has_payload = c.value() == 0xFF;

    return {};
}


}
