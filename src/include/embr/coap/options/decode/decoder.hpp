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

// DEBT: Be aware no extra help during retry (maybe you want a pubsync, delay, etc)
// otherwise we would have used the more direct delta_length_decode call
// EXPERIMENTAL, not used yet - but shaping up
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_one_ll(Streambuf& in, F&& f, uint16_t current_number, bool* has_payload)
{
    delta_length_decoder dlc;

    using r = delta_length_decoder::codes;

    auto valid = [](int c)
    {
        // Be advised, bug https://github.com/malachi-iot/estdlib/issues/220
        // presents 0xFF AS -1
        // DEBT: EOS not same as EOL or maybe EOF
        return c != 0xFF && c != -1;
    };

    // EXPERIMENTAL - too side-effecty?  Or cool?
    auto bump = [&](int& c)
    {
        c = in.sbumpc();
        // Awkwardness here because we're not caling sgetc.  But I prefer this awkwardness because
        // there's no "back up" ever, always progressing characters forward.  Fortunately it's
        // easy to consume 0xFF as long as we indicate a payload is expected
        if(c == 0xFF) *has_payload = true;
        return valid(c);
    };

    for(int c; bump(c); )
    {
        r ret = dlc.decode_byte(c);

        if(ret == r::Done)
        {
            f(dlc);
            // NOTE: Caller must consume value portion from streambuf themself including
            // advancing streambuf forward
            return {};
        }
    }

    return {};
}


template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F, class NoMatchFunctor>
estd::errc decoder<Streambuf>::decode(F&& f, bool* has_payload, NoMatchFunctor&& no_match)
{
    delta_length_decoder dlc;
    unsigned current_number = 0;

    using r = delta_length_decoder::codes;

    auto valid = [](int c)
    {
        // Be advised, bug https://github.com/malachi-iot/estdlib/issues/220
        // presents 0xFF AS -1
        // DEBT: EOS not same as EOL or maybe EOF
        return c != 0xFF && c != -1;
    };

    // EXPERIMENTAL - too side-effecty?  Or cool?
    auto bump = [&](int& c)
    {
        c = in_.sbumpc();
        // Awkwardness here because we're not caling sgetc.  But I prefer this awkwardness because
        // there's no "back up" ever, always progressing characters forward.  Fortunately it's
        // easy to consume 0xFF as long as we indicate a payload is expected
        if(c == 0xFF) *has_payload = true;
        return valid(c);
    };

    for(int c; bump(c); )
    {
        r ret = dlc.decode_byte(c);

        if(ret == r::Done)
        {
            current_number += dlc.delta();
            errc err = dispatch(
                std::forward<F>(f),
                std::forward<NoMatchFunctor>(no_match),
                (numbers)current_number, dlc.length());
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
