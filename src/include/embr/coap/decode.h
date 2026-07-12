#pragma once

#include "header.h"
#include "header/token.h"
#include "options/decode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder : public internal::policies_enum
{
    static constexpr policies policy = Presumptive;

    Streambuf in_;
    using streambuf_type = Streambuf;
    using char_type = typename streambuf_type::char_type;
    using pointer = estd::remove_const_t<char_type>*;
    using const_pointer = const char_type*;

    using options_decoder_type = options::decoder<Streambuf&>;

    options_decoder_type options_;
    header header_;

    enum states
    {
        Header,
        Token,
        Options,
        Payload
    };

    states state_{Header};

    // DEBT: Result of last read, ios style.  Expand on this
    bool good_{};

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) :
        in_{std::forward<Args>(args)...},
        options_{in_}
    {}

    constexpr bool good() const { return good_; }

    decoder& operator>>(header& h)
    {
        static_assert(policy == Presumptive);
        assert(state_ == Header);

        int read = in_.sgetn((pointer)&header_, sizeof(h));
        good_ = read == sizeof(header);
        if(good_)
        {
            h = header_;
            state_ = header_.tkl() > 0 ? Token : Options;
        }
        return *this;
    }

    decoder& operator>>(token& v)
    {
        static_assert(policy == Presumptive);
        // permit a 0-token just for ease of consumption
        assert(state_ == Token || state_ == Options);
        unsigned tkl = header_.tkl();
        if(tkl > 0)
        {
            int read = in_.sgetn((pointer)&v, tkl);
            good_ = read == tkl;
        }

        if(good_)
        {
            state_ = Options;
        }

        return *this;
    }

    /* No not gonna work well this way, need a callback flavor
    template <options::numbers n>
    options::single_decoder<n, Streambuf>& operator>>(options::option_marker<n>)
    {
        return { &options_ };
    }   */
};

}
