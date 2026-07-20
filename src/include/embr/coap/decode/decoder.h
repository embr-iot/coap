#pragma once

#include "../header.h"
#include "../header/token.h"
#include "../options/decode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder : public internal::policies_enum
{
    static constexpr policies policy = Presumptive;

    Streambuf in_;

public:
    using streambuf_type = estd::remove_cvref_t<Streambuf>;
    using char_type = typename streambuf_type::char_type;
    using pointer = estd::remove_const_t<char_type>*;
    using const_pointer = const char_type*;

    using options_decoder_type = options::decoder<Streambuf&>;

    enum states
    {
        Header,
        Token,
        Options,
        Payload,
        Done
    };

private:
    options_decoder_type options_;
    coap::header header_;

    states state_{Header};

    // DEBT: Result of last read, ios style.  Expand on this
    bool good_{};

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) :
        in_{std::forward<Args>(args)...},
        options_{in_}
    {}

    Streambuf& in() { return in_; }

    constexpr states state() const { return state_; }

    constexpr bool good() const { return good_; }

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

    template <class F>
    estd::errc options_decode(F&& f)
    {
        assert(state_ == Options);
        bool has_payload{};

        estd::errc err = options_.decode(std::forward<F>(f), &has_payload);

        if(err != estd::errc{})
        {
            good_ = false;
        }

        state_ = has_payload ? Payload : Done;
        return err;
    }

    options_decoder_type& options()
    {
        return options_;
    }

    /* No not gonna work well this way, need a callback flavor
    template <options::numbers n>
    options::single_decoder<n, Streambuf>& operator>>(options::option_marker<n>)
    {
        return { &options_ };
    }   */
};

}
