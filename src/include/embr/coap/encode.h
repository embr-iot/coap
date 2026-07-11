#pragma once

#include "fwd.h"
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
class encoder :
    public internal::encoder_base,
    public internal::streambuf_provider<Streambuf>
{
    using base_type = internal::streambuf_provider<Streambuf>;
    using this_type = encoder;

    using base_type::out_;

    // DEBT: Temporarily exposing these guys
public:
    using typename base_type::char_type;

    class options_encoder_type : public options::encoder_ll<this_type&>
    {
        using base_type = options::encoder_ll<this_type&>;

    public:
        options_encoder_type(this_type& parent) :
            base_type(parent)
        {
        }

        using base_type::operator <<;

        using payload_type = estd::detail::basic_ostream<Streambuf&>;

        //typename parent_type::payload_type& operator <<(payload_marker)
        payload_type& operator <<(payload_marker)
        {
            return base_type::provider_ << payload_marker{};
        }

    };

    template <options::numbers n>
    using options_single_encoder_type = options::single_encoder<n, options_encoder_type>;

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
        options_(*this)
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
