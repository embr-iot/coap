#pragma once

#include "fwd.h"

namespace embr::coap::options {

template <numbers n>
struct option_marker {};

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

}
