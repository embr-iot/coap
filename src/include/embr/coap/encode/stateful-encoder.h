#include "fwd.h"

#include "../internal/fwd.h"
#include "../header.h"
#include "../header/token.h"
#include "../options/encode.h"

namespace embr::coap {

/// @brief all-conditions-handled non blocking encoder
/// @remarks this may be useful to feed into above regular encoder for Retry flavor
class stateful_encoder :    // NOLINT(cppcoreguidelines-pro-type-member-init)
    public internal::encoder_base
{
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

        int written = out.sputn(static_cast<const char_type*>(data), sz);
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
    stateful_encoder() = default;

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
