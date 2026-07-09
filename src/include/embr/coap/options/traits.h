#pragma once

#include "fwd.h"

namespace embr::coap::options {

template <numbers n>
struct string_traits_base
{
    static constexpr numbers number = n;
    static constexpr auto format = value_formats::String;
    static constexpr const char* name = "N/A";
    static constexpr bool multi = false;
};

template <numbers n>
struct uint_traits_base
{
    static constexpr numbers number = n;
    static constexpr auto format = value_formats::Uint;
    static constexpr const char* name = "N/A";
    static constexpr bool multi = false;
};

template <>
struct option_traits<numbers::UriPath> : string_traits_base<numbers::UriPath>
{
    static constexpr const char* name = "Uri-Path";

    static constexpr bool multi = true;
};

template <>
struct option_traits<numbers::UriHost> : string_traits_base<numbers::UriHost>
{
    static constexpr const char* name = "Uri-Host";
};

template <>
struct option_traits<numbers::UriQuery> : string_traits_base<numbers::UriQuery>
{
    static constexpr const char* name = "Uri-Query";

    static constexpr bool multi = true;
};

template <>
struct option_traits<numbers::ContentFormat> : uint_traits_base<numbers::ContentFormat>
{
    static constexpr const char* name = "Content-Format";
};

}
