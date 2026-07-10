#pragma once

#include "decode/decoder.hpp"
#include "decode/stateful.hpp"

#include "fwd.h"
#include "markers.h"

#include <estd/cstdint.h>
#include <estd/istream.h>

namespace embr::coap::options {


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
