#pragma once

#include <estd/ostream.h>
#include <estd/string_view.h>

#include "../internal/streambuf.h"
#include "encode/stateful.h"

#include "fwd.h"
#include "../fwd.h"
#include "option.h"
#include "markers.h"
#include "traits.h"

namespace embr::coap::options {

/*
constexpr option<numbers::UriPath> uri_path(const char* path)
{
    return { .length = (unsigned)estd::char_traits<char>::length(path), .string = path };
}
*/

/* Not ready yet
template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class payload_encoder : public estd::detail::basic_ostream<Streambuf>
{
    using base_type = estd::detail::basic_ostream<Streambuf>;
    using streambuf_type = typename estd::remove_reference_t<Streambuf>;
    using encoder_type = encoder<streambuf_type>;

    encoder_type* parent_;

public:
    payload_encoder(encoder_type* parent) :
        base_type{parent->out_},
        parent_{parent}
    {}
};  */

// DEBT: Operating in Presumptive mode all the time.  Import in and add support for
// NonContiguous and Retry mode

template <numbers n, class Parent>
class single_encoder
{
public:
    using traits = option_traits<n>;
    using parent_type = Parent;
    using streambuf_type = typename parent_type::streambuf_type;
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    parent_type* parent_;

    // TODO: Consider a single_encoder<bool multi> which compile-time prohibits doing
    // say multiple hosts or multiple content-types, etc
//public:

    single_encoder operator<<(unsigned)
    {
        static_assert(traits::format == value_formats::Uint);
        return *this;
    }

    single_encoder operator<<(estd::string_view string)
    {
        static_assert(traits::format == value_formats::String);
        parent_->write_header(n, string.size());
        parent_->provider_.out().xsputn(reinterpret_cast<const_pointer>(string.data()), string.size());
        return *this;
    }

    template <numbers number>
    single_encoder<number, Parent> operator<<(option_marker<number>)
    {
        return { parent_ };
    }

    parent_type& operator<<(payload_marker)
    {
        //static_assert(has_grandparent, "Payload marker only supported in full-encoder mode");

        return *parent_ << payload_marker{};
    }
};


template <class StreambufProvider>
class encoder_ll
{
#if UNIT_TESTING
public:
#endif
    unsigned current_number_{};

    using this_type = encoder_ll;

    StreambufProvider provider_;

    using provider_type = typename estd::remove_reference_t<StreambufProvider>;
    using streambuf_type = typename provider_type::streambuf_type;

    template <numbers n, class Parent>
    friend class single_encoder;

    //template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf2>
    //friend class payload_encoder;

public:
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    template <numbers n>
    using single_encoder_type = single_encoder<n, this_type>;

    static_assert(sizeof(char_type) == 1);

    const streambuf_type& out() const { return provider_.out(); }

protected:
    template <class ...Args>
    explicit constexpr encoder_ll(Args&&...args) :
        provider_{std::forward<Args>(args)...}
    {}


public:
    void write_header(numbers number, unsigned length)
    {
        auto out = reinterpret_cast<uint8_t*>(provider_.out().pptr());
        auto end = reinterpret_cast<uint8_t*>(provider_.out().epptr());

        // option header part can take up to 5 bytes
        assert(end - out >= 5);

        // DEBT: Do intermediate buffer flavor if we are in blocking mode and
        // don't have full 5 bytes available

        end = delta_length_encode(out, current_number_, number, length);

        current_number_ = number;

        provider_.out().pubseekoff(end - out, estd::ios_base::cur);
    }

    template <numbers n>
    encoder_ll& operator <<(option<n> oh)
    {
        write_header(oh.number, oh.length);

        return *this;
    }

    template <numbers n>
    single_encoder_type<n> operator<<(option_marker<n>)
    {
        return { this };
    }
};

// Specifically NOT using alias to help slightly tidy up C++ error volume
template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder : public encoder_ll<internal::streambuf_provider<Streambuf>>
{
    using base_type = encoder_ll<internal::streambuf_provider<Streambuf>>;

public:
    template <class ...Args>
    constexpr explicit encoder(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

    using base_type::operator <<;

    // child_encoder needed for this
    encoder& operator<<(payload_marker) = delete;
};

}
