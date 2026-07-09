#pragma once

#include <estd/ostream.h>
#include <estd/string_view.h>

#include "fwd.h"
#include "../fwd.h"
#include "traits.h"

namespace embr::coap::options {

template <numbers n>
struct option_marker {};

template <numbers n>
struct option
{
    static constexpr numbers number = n;

    unsigned length;
    union
    {
        const uint8_t* opaque;
        unsigned uint;
        const char* string;
    };
};

/*
constexpr option<numbers::UriPath> uri_path(const char* path)
{
    return { .length = (unsigned)estd::char_traits<char>::length(path), .string = path };
}
*/

// Operate like std cout manipulators
inline namespace markers {

inline constexpr option_marker<numbers::ContentFormat> content_format;
inline constexpr option_marker<numbers::UriHost> uri_host;
inline constexpr option_marker<numbers::UriPath> uri_path;
inline constexpr option_marker<numbers::UriQuery> uri_query;
inline constexpr option_marker<numbers::Observe> observe;
inline constexpr option_marker<numbers::Echo> echo;
inline constexpr option_marker<numbers::ProxyUri> proxy_uri;

}

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

template <numbers n, ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf, bool has_grandparent = false>
class single_encoder
{
public:
    using traits = option_traits<n>;
    using encoder_type = estd::conditional_t<
        has_grandparent,
        child_encoder<Streambuf>,
        encoder<Streambuf>>;
    using streambuf_type = typename estd::remove_reference_t<Streambuf>;
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    encoder_type* parent_;

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
        parent_->out_.xsputn(reinterpret_cast<const_pointer>(string.data()), string.size());
        return *this;
    }

    template <numbers number>
    single_encoder<number, Streambuf> operator<<(option_marker<number>)
    {
        return { parent_ };
    }

    encoder_type& operator<<(payload_marker)
    {
        static_assert(has_grandparent, "Payload marker only supported in full-encoder mode");

        return *parent_ << payload_marker{};
    }
};


template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder
{
#if UNIT_TESTING
public:
#endif
    unsigned current_number_{};
    Streambuf out_;

    using streambuf_type = typename estd::remove_reference_t<Streambuf>;

    template <numbers n, ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf2, bool has_grandparent>
    friend class single_encoder;

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf2>
    friend class payload_encoder;

    //using payload_type = payload_encoder<Streambuf&>;
    using payload_type = estd::detail::basic_ostream<Streambuf&>;

public:
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    static_assert(sizeof(char_type) == 1);

    const Streambuf& out() const { return out_; }

    template <class ...Args>
    explicit constexpr encoder(Args&&...args) : out_{std::forward<Args>(args)...}
    {}

    void write_header(numbers number, unsigned length)
    {
        auto out = reinterpret_cast<uint8_t*>(out_.pptr());
        auto end = reinterpret_cast<uint8_t*>(out_.epptr());

        // option header part can take up to 5 bytes
        assert(end - out >= 5);

        // DEBT: Do intermediate buffer flavor if we are in blocking mode and
        // don't have full 5 bytes available

        end = delta_length_encode(out, current_number_, number, length);

        out_.pubseekoff(end - out, estd::ios_base::cur);
    }

    template <numbers n>
    encoder& operator <<(option<n> oh)
    {
        write_header(oh.number, oh.length);

        return *this;
    }

    template <numbers n>
    single_encoder<n, Streambuf> operator<<(option_marker<n>)
    {
        return { this };
    }

    // child_encoder needed for this
    encoder& operator<<(payload_marker) = delete;
};

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class child_encoder : public encoder<Streambuf&>
{
    using base_type = encoder<Streambuf&>;
    using parent_type = coap::encoder<Streambuf>;

    parent_type* parent_;

public:
    child_encoder(parent_type* parent) : base_type(parent->out_) {}

    using base_type::operator <<;

    typename parent_type::payload_type& operator <<(payload_marker)
    {
        return *parent_ << payload_marker{};
    }

};

}
