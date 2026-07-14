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
    public internal::policies_enum,
    public internal::streambuf_provider<Streambuf>
{
    using base_type = internal::streambuf_provider<Streambuf>;
    using this_type = encoder;

    using base_type::out_;

    // DEBT: Temporarily exposing these guys
public:
    using typename base_type::const_pointer;

    //using payload_type = options::payload_encoder<Streambuf&>;
    using payload_type = estd::detail::basic_ostream<Streambuf&>;

    class options_encoder_type : public options::encoder_ll<this_type&>
    {
        using base_type = options::encoder_ll<this_type&>;

    public:
        constexpr explicit options_encoder_type(this_type& parent) :
            base_type(parent)
        {
        }

        using base_type::operator <<;

        payload_type& operator <<(payload_marker)
        {
            return base_type::provider_ << payload_marker{};
        }

    };

    options_encoder_type options_;

    payload_type payload_{out_};

    // DEBT: Once estd 0.8.12 is fully available use this variant in place of the freestanding members
    using storage_type = estd::variant<
        estd::monostate, header, options_encoder_type, payload_type>;

    unsigned token_length_{};

public:
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
    options::single_encoder<n, options_encoder_type> operator<<(options::option_marker<n>)
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


/// @brief all-conditions-handled non blocking encoder
/// @remarks this may be useful to feed into above regular encoder for Retry flavor
class stateful_encoder
{
    enum states
    {
        Header,
        Token,
        Options,
        Payload
    };

    states state_{Header};

    // For header, then token
    union
    {
        internal::out_accumulator<8> temp_;
        options::stateful_encoder options_;
    };

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool sputn(Streambuf& out, const void* data, unsigned sz, states transition_to)
    {
        using char_type = typename Streambuf::char_type;

        int written = out.sputn((const char_type*)data, sz);
        if(written == sz)
        {
            state_ = transition_to;
            return true;
        }
        temp_.init(data, sz - written);
        return false;
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool sputn(Streambuf& out, states transition_to)
    {
        if(!temp_.sputn(out))   return false;

        state_ = transition_to;
        return true;
    }

public:
    // true = state change occurred

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool header(Streambuf& out, const coap::header& v)
    {
        assert(state_ == Header);

        return sputn(out, &v, sizeof(v), v.tkl() > 0 ? Token : Options);
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool header(Streambuf& out)
    {
        assert(state_ == Header);

        return sputn(out, Token);
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool token(Streambuf& out, const uint8_t* v, unsigned sz)
    {
        return sputn(out, v, sz, Options);
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool token(Streambuf& out)
    {
        return sputn(out, Options);
    }

    // EXPERIMENTAL
    // If state change didn't occur right away, one may call this guy
    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool poll_one(Streambuf& out)
    {
        if(state_ == Header)
        {
            return header(out);
        }
        else if(state_ == Token)
        {
            return token(out);
        }

        return {};
    }

    options::stateful_encoder& options()
    {
        assert(state_ == Options);

        // FIX: Init this once on a state change, options() accessor
        // might be called many times
        new (&options_) options::stateful_encoder(estd::nullopt);
        return options_;
    }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool payload(Streambuf& out)
    {
        using traits = typename Streambuf::traits_type;

        return traits::not_eof(out.sputc(0xFF));
    }

    // You're on your own for payload content, friend
};

}
