#pragma once

#include "decode/decoder.hpp"
#include "decode/stateful.hpp"

#include "../decode/breadcrumb.h"   // EXPERIMENTAL

#include "fwd.h"
#include "markers.h"

#include <estd/cstdint.h>
#include <estd/internal/macros.h>
#include <estd/istream.h>

namespace embr::coap::options {

// EXPERIMENTAL
class cooked_decoder
{
    using bc = embr::internal::breadcrumb;
    using bc_traits = embr::internal::breadcrumb_traits<bc>;

    using optional_type = estd::layer1::optional<content_formats, (content_formats)0xFFFF>;

    optional_type accept_;
    optional_type content_format_;
    breadcrumb_matcher<bc> uri_;

public:
    constexpr explicit cooked_decoder(const bc* nav) :
        uri_(nav)
    {}

    template <estd::size_t N>
    constexpr explicit cooked_decoder(const bc (&nav)[N]) :
        uri_{nav}
    {
    }

    template <numbers n>
    void investigate(const option<n>& option)
    {
        if constexpr(option.number == numbers::UriPath)
        {
            uri_.search(option.string());
        }
        else if constexpr(option.number == numbers::Accept)
        {
            accept_ = static_cast<content_formats>(option.uint());
        }
        else if constexpr(option.number == numbers::ContentFormat)
        {
            content_format_ = static_cast<content_formats>(option.uint());
        }
    }

    const optional_type& accept() const { return accept_; }
    const optional_type& content_format() const { return content_format_; }

    // DEBT: This would be better, not there yet
    //const estd::layer1::optional<int, -1> uri() { return uri_.id(); }
    const estd::optional<int> uri() { return uri_.id(); }
};

/*
template <numbers n, ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, bool has_grandparent = false>
class single_decoder
{
    using decoder_type = decoder<Streambuf>;
    using streambuf_type = typename estd::remove_reference_t<Streambuf>;

    decoder_type* parent_;

public:
    constexpr single_decoder(decoder_type* decoder) :
        parent_{decoder}
    {}
};  */

// IDEA CAPTURE
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_with_retry();

// IDEA CAPTURE
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_noncontiguous();

// Almost works, still experimental - in c++17 too opportunistically eats up
// things that you'd hope estd::span would resolve
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc decode_exp(Streambuf& in, F&& f)
{
    decoder<Streambuf&> d(in);

    return d.decode(std::forward<F>(f));
}

// UNTESTED
template <class T, class F>
estd::errc decode(estd::span<T> in, F&& f)
{
    using streambuf_type = estd::detail::basic_ispanbuf<T>;

    decoder<streambuf_type> d(in);

    return d.decode(std::forward<F>(f));
}

}
