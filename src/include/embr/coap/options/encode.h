#pragma once

#include <estd/ostream.h>
#include <estd/string_view.h>

#include "fwd.h"

namespace embr::coap::options {

// One-shot flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
uint8_t* delta_length_encode(uint8_t* out, unsigned current_number, numbers number, unsigned length);

template <numbers n>
struct string_traits_base
{
    static constexpr numbers number = n;
    static constexpr auto format = value_formats::String;
    static constexpr const char* name = "N/A";
    static constexpr bool multi = false;
};

template <numbers n>
struct option_traits;

template <>
struct option_traits<numbers::UriPath> : string_traits_base<numbers::UriPath>
{
    static constexpr const char* name = "UriPath";

    static constexpr bool multi = true;
};

template <>
struct option_traits<numbers::UriHost> : string_traits_base<numbers::UriHost>
{
    static constexpr const char* name = "UriHost";
};

template <numbers n>
struct option_marker
{

};

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

inline constexpr option_marker<numbers::UriHost> uri_host;
inline constexpr option_marker<numbers::UriPath> uri_path;

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder;


template <numbers n, ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class single_encoder
{
public:
    using traits = option_traits<n>;

    encoder<Streambuf>* parent_;

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
        parent_->out_.xsputn((const uint8_t*)string.data(), string.size());
        return *this;
    }

    single_encoder operator/(estd::string_view string)
    {
        static_assert(traits::format == value_formats::String);
        parent_->write_header(n, string.size());
        parent_->out_.xsputn((const uint8_t*)string.data(), string.size());
        return { parent_ };
    }

    template <numbers number>
    single_encoder<number, Streambuf> operator<<(option_marker<number>)
    {
        return { parent_ };
    }
};

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder
{
    unsigned current_number_{};
    Streambuf out_;

    template <numbers n, ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf2>
    friend class single_encoder;

public:
    const Streambuf& out() const { return out_; }

    template <class ...Args>
    explicit constexpr encoder(Args&&...args) : out_{std::forward<Args>(args)...}
    {}

    void write_header(numbers number, unsigned length)
    {
        auto out = out_.pptr();
        auto end = out_.epptr();

        // option header part can take up to 5 bytes
        assert(end - out >= 5);

        end = delta_length_encode(out, current_number_, number, length);

        out_.pubseekoff(end - out, estd::ios_base::cur);
    }

    template <numbers n>
    encoder& operator <<(option<n> oh)
    {
        write(oh.number, oh.length);

        return *this;
    }

    // DEBT: If we can, take this into mutator category so we can get rid of trailing ()
    template <numbers n>
    single_encoder<n, Streambuf> operator<<(option_marker<n>)
    {
        return { this };
    }
};

}
