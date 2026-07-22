#pragma once

#include "decoder.h"
#include "../numbers.h"
#include "delta-length-decoder.h"
#include "delta-length-decode.hpp"

#include "../traits.h"
#include "../option.h"
#include "../../uint.h"
#include "../../internal/accumulator.h"

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <class F>
errc decoder<Streambuf, Traits>::emit(F&& f, numbers number, unsigned len)
{
    // TODO: Strongly consider 'Presumptive' mode auto-advancing
    // through buf on your behalf

    const unsigned avail = in_.in_avail();

    option o;

    o.number = number;
    o.length = len;

    if(avail < len)
    {
        if constexpr(policy == Presumptive) return errc::bad;

        // Signal that consumer should look directly at streambuf
        o.opaque_ = nullptr;
    }
    else
    {
        o.opaque_ = in_.gptr();
    }

    // Be advised also you must pubseekoff (or similar) yourself
    f(o);

    return errc{};
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <numbers number, class F>
errc decoder<Streambuf, Traits>::emit(F&& f, unsigned len, const uint8_t* data)
{
    using traits = option_traits<number>;
    option<number> o;

    o.length = len;
    o.value(data);

    f(o);

    // invalid_argument can be considered a warning, not an error
    if(len < traits::min_length)
    {
        return errc::warn;
    }
    else if(traits::max_length != 0 && len > traits::max_length)
    {
        return errc::warn;
    }

    return {};
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <numbers number, class F, class Retry2>
errc decoder<Streambuf, Traits>::dispatch_sgetn_ll(F&& f, unsigned len, Retry2&& retry)
{
    constexpr unsigned max_size = traits::in_accumulator_size;
    internal::in_accumulator<max_size> accumulator;

    if(len > max_size) return errc::fail;

    accumulator.init(len);

    while(accumulator.sgetn(in_) == false)
    {
        retry();
    }

    return emit<number>(std::forward<F>(f), len, accumulator.buf_);
}


template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <numbers number, class F, class Retry2>
errc decoder<Streambuf, Traits>::dispatch_gptr_ll(F&& f, unsigned len, Retry2&& retry)
{
    const unsigned avail = in_.in_avail();

    static_assert(streambuf_policy::use::gptr >= rfc::may);

    if constexpr(policy == Presumptive)
    {
        if(avail < len)
            // Input stream is presumed to be incorrectly truncated in this mode.
            // Since it is akin to user input, this is not an assert but a validation class
            // error
            return errc::bad;
    }
    else if constexpr(policy == Retry || policy == NonContiguous)
    {
        if(avail < len)
            return dispatch_sgetn_ll(std::forward<F>(f), len, std::forward<Retry2>(retry));
    }
    else
        static_assert(false, "Unrecognized policy");

    return emit<number>(std::forward<F>(f), len, (const uint8_t*)in_.gptr());
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <class F, class NoMatchFunctor, class Retry2>
errc decoder<Streambuf, Traits>::dispatch_ll(F&& f, NoMatchFunctor&& no_match, numbers number, unsigned len, Retry2&& retry)
{
    errc err = dispatch_number_ll(number,
        [&](auto number)
        {
            return dispatch_gptr_ll<number>(std::forward<F>(f), len, std::forward<Retry2>(retry));
        },
        [&](numbers)
        {
            // no_match path is optional, oftentimes we don't care much
            if constexpr(!estd::is_same_v<NoMatchFunctor, estd::monostate>)
            {
                emit(std::forward<NoMatchFunctor>(no_match), number, len);
                return errc{};
            }

            // In the case where we are ignoring unmatched arguments, indicate
            // a warning state
            return errc::warn;
        });

    // DEBT: Make sure this proceeds forward correctly
    in_.pubseekoff(len, estd::ios_base::cur);

    return err;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <class F, class NoMatchFunctor>
errc decoder<Streambuf, Traits>::dispatch(F&& f, bool* has_payload, NoMatchFunctor&& no_match)
{
    errc err{};
    unsigned current_number = 0;

    auto f2 = [&](const delta_length_decoder& dld)
    {
        current_number += dld.delta();
        errc err2 = dispatch_ll(
            std::forward<F>(f),
            std::forward<NoMatchFunctor>(no_match),
            (numbers)current_number, dld.length());

        if(err2 == errc::bad)    err = err2;
    };

    estd::optional<int> c;

    while(!(c = delta_length_decode(in_, f2)).has_value())
    {
    }

    *has_payload = c.value() == 0xFF;

    return err;
}


// UNTESTED
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <class F>
errc decoder<Streambuf, Traits>::decode(F&& f, bool* has_payload)
{
    errc err{};
    unsigned current_number = 0;

    auto f2 = [&](const delta_length_decoder& dld)
    {
        current_number += dld.delta();
        // DEBT:  Heed emit err code
        emit(std::forward<F>(f), (numbers)current_number, dld.length());
    };

    estd::optional<int> c;

    while(!(c = delta_length_decode(in_, f2)).has_value())
    {
    }

    *has_payload = c.value() == 0xFF;

    return err;
}

}
