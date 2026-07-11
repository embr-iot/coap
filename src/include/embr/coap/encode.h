#pragma once

#include "header.h"
#include "header/token.h"
#include "options/encode.h"

namespace embr::coap {

namespace internal {

struct encoder_base
{
    enum States
    {
        Header,
        Token,
        Options,
        Payload
    };

#if !UNIT_TESTING
protected:
#endif

    States state_{Header};

public:
    constexpr States state() const { return state_; }
};


}

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder : public internal::encoder_base, internal::streambuf_provider<Streambuf>
{
    using base_type = internal::streambuf_provider<Streambuf>;

    // DEBT: Temporarily exposing these guys
public:
    using base_type::out_;
    using typename base_type::char_type;

    using options_encoder_type = options::child_encoder<Streambuf>;

    template <options::numbers n>
    using options_single_encoder_type = options::single_encoder<n, Streambuf, true>;

    options_encoder_type options_;

    //using payload_type = options::payload_encoder<Streambuf&>;
    using payload_type = estd::detail::basic_ostream<Streambuf&>;

    payload_type payload_{out_};

    unsigned token_length_{};

public:
    using const_pointer = const char_type*;

    template <class ...Args>
    constexpr explicit encoder(Args&&... args) :
        base_type(std::forward<Args>(args)...),
        options_(this)
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
    options_single_encoder_type<n> operator<<(options::option_marker<n>)
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
