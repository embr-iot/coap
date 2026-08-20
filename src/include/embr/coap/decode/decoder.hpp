#pragma once

#include "decoder.h"
#include "../options/decode/stateful.hpp"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
void decoder<Streambuf>::read_header()
{
    static_assert(policy == Presumptive);
    assert(state_ == Header);

    int read = in_.sgetn((pointer)&header_, sizeof(header_));
    good_ = read == sizeof(coap::header);
    good_ &= header_.invariant();

    if(good_) state_ = Token;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::init_options() -> decoder&
{
    current_number_ = 0;
    state_ = Options;
    return *this;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::operator>>(token& v) -> decoder&
{
    static_assert(policy == Presumptive);
    assert(state_ == Token);

    const unsigned tkl = header_.tkl();

    // permit a 0-token just for ease of consumption
    v.size = tkl;

    if(tkl == 0)    return init_options();

    int read = in_.sgetn((pointer)&v, tkl);
    good_ = read == tkl;

    // Just in case they go >> flavor
    if(good_)   init_options();

    return *this;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
template <class F>
errc decoder<Streambuf>::options_decode(F&& f)
{
    if(state_ == Token) init_options();

    assert(state_ == Options);

    bool has_payload{};
    options::decoder<Streambuf&, traits> options(in_);

    errc err = options.dispatch(std::forward<F>(f), &has_payload);

    good_ = err == errc{};

    state_ = has_payload ? Payload : Done;
    return err;
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
auto decoder<Streambuf>::operator>>(options::option<>& v) -> decoder&
{
    if(!good_)  return *this;

    if(state_ == Token)
    {
        // Silently consume Token if that's what consumer really wants
        // DEBT: Consider a policy where token MUST be read
        // DEBT: Do a flavor of this which doesn't require a temporary token on the stack
        token temp;
        *this >> temp;
        //good_ = header_.tkl() == 0;
        //current_number_ = 0;
        //state_ = Options;
    }
    else
        assert(state_ == Options);

    using options_decoder = options::decoder<Streambuf&, traits>;

    errc err = options_decoder::decode_one(in_, &v, &current_number_);

    switch(err)
    {
        case errc{}:  // NOLINT
        {
            // Peek ahead to determine if we're EOF (Done) or seeing a payload marker
            // DEBT: I wish we didn't have to peek, but this seems to be the best experience
            switch(in_.sgetc())
            {
                case 0xFF:
                    state_ = Payload;
                    break;

                case (int)char_traits::eof():
                    state_ = Done;
                    break;

            }
            break;
        }

        /*
         * Since we lookahead, then encountering these is actually an error since state_
         * should disallow it
        case errc::warn:
            state_ = Done;
            break;

        case errc::alternate:
            state_ = Payload;
            break;  */

        // 'alternate' means we unexpectedly hit payload, which we already should have
        // detected in errc{} above.
        // 'bad' is just how it sounds
        default:
            good_ = false;
            break;
    }

    return *this;
}

}
