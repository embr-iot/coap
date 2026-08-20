#pragma once

#include "../header.h"
#include "../header/token.h"
#include "../options/decode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder : public internal::policies_enum
{
    using traits = options::presumptive_decoder_traits;
    static constexpr policies policy = traits::policy;

    Streambuf in_;

public:
    using streambuf_type = estd::remove_cvref_t<Streambuf>;
    using char_type = typename streambuf_type::char_type;
    using char_traits = typename streambuf_type::traits_type;
    using pointer = estd::remove_const_t<char_type>*;
    using const_pointer = const char_type*;

    enum states
    {
        Header,
        Token,
        Options,
        Payload,
        Done
    };

private:
    union
    {
        coap::header header_;
        uint16_t current_number_;
    };

    states state_{Header};

    // DEBT: Result of last read, ios style.  Expand on this
    bool good_ : 1;

    decoder& init_options();

public:
    template <class ...Args>
    explicit constexpr decoder(Args&&...args) :
        in_{std::forward<Args>(args)...},
        good_{}
    {}

    Streambuf& in() { return in_; }

    constexpr states state() const { return state_; }

    constexpr bool good() const { return good_; }

    // istream style
    constexpr explicit operator bool() const { return good_; }

    const coap::header& header() const { return header_; }

    // Give consumer option of using our cached header directly
    void read_header();

    decoder& operator>>(coap::header& h)
    {
        read_header();

        if(good_)   h = header_;

        return *this;
    }

    decoder& operator>>(token& v);

    decoder& operator>>(options::option<>&);

    template <class F>
    errc options_decode(F&& f);

    /* No not gonna work well this way, need a callback flavor
    template <options::numbers n>
    options::single_decoder<n, Streambuf>& operator>>(options::option_marker<n>)
    {
        return { &options_ };
    }   */
};

}
