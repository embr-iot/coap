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

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
constexpr option<> make_option(Streambuf& in, numbers number, unsigned len)
{
    using use = typename Streambuf::policy::use;
    using rfc = estd::internal::rfc2119;

    static_assert(use::gptr >= rfc::may);

    return { number, len, in.in_avail() < len ? nullptr : (const uint8_t*)in.gptr() };
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <class F>
errc decoder<Streambuf, Traits>::emit(F&& f, const option<>& o)
{
    if(o.length > 0 && o.opaque_ == nullptr)
        if constexpr(policy == Presumptive) return errc::bad;

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

// NOT USED OR TESTED YET
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
                emit(std::forward<NoMatchFunctor>(no_match), make_option(in_, number, len));
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

    // no value means successful decode without seeing a payload
    while(!(c = delta_length_decode(in_, f2)).has_value())
    {
    }

    *has_payload = c.value() == 0xFF;

    return err;
}


// Does not advance stream past value portion, thus the ll
// This guy requires Presumptive or NonContiguous
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
errc decode_one_ll(Streambuf& in, option<>* opt, uint16_t* current_number)
{
    using char_traits = typename Streambuf::traits_type;

    delta_length_decoder dld;

    estd::optional<int> c = delta_length_decode(dld, in);

    // Since we gently expect option data here, no further data is a warning, but not corruption per se
    if(c == char_traits::eof()) return errc::warn;
    if(c == -2) return errc::bad;

    *current_number += dld.delta();

    *opt = make_option(in, (numbers)*current_number, dld.length());

    // We use 'alternate' to signal more data is available in the form of a payload and that no
    // option was actually available.  This fits the lower level nature of decode_one better
    // than a payload flag because:
    // - Usually we're in a position to to sgetc to notice if a payload is present
    // - Usually we're wrapped up in internal calls here, so checking against cycle is not unnnatural
    if(c == 0xFF) return errc::alternate;

    return {};
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
errc decoder<Streambuf, Traits>::decode_one(option<>* o, uint16_t* current_number)
{
    errc err = decode_one_ll(in_, o, current_number);

    if(err != errc{})   return err;

    if constexpr(policy == Presumptive)
    {
        // NOTE: Presumptive it is safe to seek here, and decode_one_ll populates 'opaque' with any data
        // (event uint, etc) if it's present.  Retry/NonContiguous you are on your own

        pos_type ret = in_.pubseekoff(o->length, estd::ios_base::cur);

        if(ret == -1) err = errc::bad;
    }
    return err;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
template <class F>
errc decoder<Streambuf, Traits>::decode(F&& f)
{
    uint16_t current_number = 0;
    option o;   // NOLINT - uninitialized is OK here, decode_one does that
    errc err;

    while((err = decode_one(&o, &current_number)) == errc{})
        f(o);

    return err;
}

}
