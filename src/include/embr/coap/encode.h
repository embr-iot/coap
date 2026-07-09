#pragma once

#include "header.h"
#include "header/token.h"
#include "options/encode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder
{
    // DEBT: Temporarily exposing these guys
public:
    using options_encoder_type = options::child_encoder<Streambuf>;

    Streambuf out_;
    using streambuf_type = Streambuf;

    // FIX: Needs to be child_encoder
    options::encoder<Streambuf&> options_{out_};
    //options_encoder_type options_;

    //using payload_type = options::payload_encoder<Streambuf&>;
    using payload_type = estd::detail::basic_ostream<Streambuf&>;

    payload_type payload_{out_};

    unsigned token_length_{};

    enum States
    {
        Header,
        Token,
        Options,
        Payload
    };

    States state_{Header};

public:
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    template <class ...Args>
    constexpr explicit encoder(Args&&... args) :
        out_(std::forward<Args>(args)...)
        //,options_(this)
    {}

    encoder& operator<<(const header& v)
    {
        assert(state_ == Header);

        // DEBT: Account for errors here
        out_.sputn((const_pointer)&v, sizeof(header));
        token_length_ = v.tkl();

        state_ = token_length_ > 0 ? Token : Options;
        return *this;
    }

    encoder& operator<<(const token& v)
    {
        assert(state_ == Token);
        assert(v.size == 0 || v.size == token_length_);

        // DEBT: Account for errors here
        out_.sputn((const_pointer)v.value, token_length_);

        state_ == Options;
        return *this;
    }

    template <options::numbers n>
    options::single_encoder<n, Streambuf&> operator<<(options::option_marker<n>)
    {
        assert(state_ == Options);
        return { &options_ };
    }

    payload_type& operator<<(payload_marker)
    {
        assert(state_ == Options);
        state_ = Payload;
        out_.sputc(0xFF);
        return payload_;
    }
};

}
