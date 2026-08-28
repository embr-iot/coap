#pragma once

#include "fwd.h"

namespace embr::coap::options {

template <numbers n>
struct options_traits_base
{
    static constexpr numbers number = n;
    static constexpr const char* name = "N/A";
    static constexpr unsigned min_length = 0;
    static constexpr unsigned max_length = 0;
    static constexpr auto format = value_formats::Empty;

    // As per https://datatracker.ietf.org/doc/html/rfc7252#section-5.4.6
    static constexpr bool critical = n & 1;
    static constexpr bool unsafe = n & 2;
    static constexpr bool no_cache_key = ((n & 0x1e) == 0x1c);
    static constexpr bool repeatable = false;
};

template <numbers n>
struct opaque_traits_base : options_traits_base<n>
{
    static constexpr auto format = value_formats::Opaque;
};

template <numbers n>
struct string_traits_base : options_traits_base<n>
{
    static constexpr auto format = value_formats::String;
};

template <numbers n>
struct uint_traits_base : options_traits_base<n>
{
    static constexpr auto format = value_formats::Uint;
};

template <>
struct traits<numbers::Accept> : uint_traits_base<numbers::Accept>
{
    static constexpr const char* name = "Accept";

    static constexpr unsigned max_length = 2;
};

template <>
struct traits<numbers::Block1> : uint_traits_base<numbers::Block1>
{
    static constexpr const char* name = "Block1";

    static constexpr unsigned max_length = 3;
};

template <>
struct traits<numbers::Block2> : uint_traits_base<numbers::Block2>
{
    static constexpr const char* name = "Block2";

    static constexpr unsigned max_length = 3;
};

template <>
struct traits<numbers::ContentFormat> : uint_traits_base<numbers::ContentFormat>
{
    static constexpr const char* name = "Content-Format";

    static constexpr unsigned max_length = 2;
};

template <>
struct traits<numbers::Echo> : opaque_traits_base<numbers::Echo>
{
    static constexpr const char* name = "Echo";

    static constexpr unsigned min_length = 1;
    static constexpr unsigned max_length = 40;
};

template <>
struct traits<numbers::ETag> : opaque_traits_base<numbers::ETag>
{
    static constexpr const char* name = "ETag";

    static constexpr unsigned min_length = 1;
    static constexpr unsigned max_length = 8;

    static constexpr bool repeatable = true;
};


template <>
struct traits<numbers::LocationPath> : string_traits_base<numbers::LocationPath>
{
    static constexpr const char* name = "Location-Path";
};

template <>
struct traits<numbers::LocationQuery> : string_traits_base<numbers::LocationQuery>
{
    static constexpr const char* name = "Location-Query";
};

template <>
struct traits<numbers::MaxAge> : uint_traits_base<numbers::MaxAge>
{
    static constexpr const char* name = "Max-Age";

    static constexpr unsigned max_length = 4;
};

template <>
struct traits<numbers::ProxyUri> : string_traits_base<numbers::ProxyUri>
{
    static constexpr const char* name = "Proxy-Uri";

    static constexpr unsigned min_length = 1;
    static constexpr unsigned max_length = 1034;
};

template <>
struct traits<numbers::ProxyScheme> : string_traits_base<numbers::ProxyScheme>
{
    static constexpr const char* name = "Proxy-Scheme";

    static constexpr unsigned min_length = 1;
    static constexpr unsigned max_length = 255;
};

template <>
struct traits<numbers::Size1> : uint_traits_base<numbers::Size1>
{
    static constexpr const char* name = "Size1";

    static constexpr unsigned max_length = 4;
};

template <>
struct traits<numbers::Size2> : uint_traits_base<numbers::Size2>
{
    static constexpr const char* name = "Size2";
};

template <>
struct traits<numbers::UriPath> : string_traits_base<numbers::UriPath>
{
    static constexpr const char* name = "Uri-Path";

    static constexpr bool repeatable = true;
};

template <>
struct traits<numbers::UriPort> : uint_traits_base<numbers::UriPort>
{
    static constexpr const char* name = "Uri-Port";

    static constexpr unsigned max_length = 2;
};

template <>
struct traits<numbers::UriHost> : string_traits_base<numbers::UriHost>
{
    static constexpr const char* name = "Uri-Host";
};

template <>
struct traits<numbers::UriQuery> : string_traits_base<numbers::UriQuery>
{
    static constexpr const char* name = "Uri-Query";

    static constexpr bool repeatable = true;
};

}
