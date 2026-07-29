#pragma once

#include "base.h"

#include "../header.h"
#include "../header/token.h"
#include "../options/encode.h"

#include <estd/ostream.h>

namespace embr::coap {

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

        state_ = Options;
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
    
}
