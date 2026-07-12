#pragma once

#include "fwd.h"

// For monostate
#include <estd/variant.h>

// These options can be disabled to save some ROM space if those features
// aren't used.  Remains to be seen just how much ROM they use, though

// DEBT: Place feature flags elsewhere
#ifndef FEATURE_EMBR_COAP_OPTION_ECHO
#define FEATURE_EMBR_COAP_OPTION_ECHO 1
#endif

#ifndef FEATURE_EMBR_COAP_OPTION_BLOCK
#define FEATURE_EMBR_COAP_OPTION_BLOCK 1
#endif

namespace embr::coap::options {

template <numbers n>
using number_constant = estd::integral_constant<numbers, n>;

template <class F, class F2, class ...Args>
auto dispatch_number_ll(numbers number, F&& f, F2&& no_match, Args&&...args)
{
    using n = numbers;

    switch(number)
    {
        case n::Accept:
            return f(number_constant<n::Accept>{}, std::forward<Args>(args)...);

#ifndef FEATURE_EMBR_COAP_OPTION_BLOCK
        case n::Block1:
            return f(number_constant<n::Block1>{}, std::forward<Args>(args)...);

        case n::Block2:
            return f(number_constant<n::Block2>{}, std::forward<Args>(args)...);
#endif

        case n::ContentFormat:
            return f(number_constant<n::ContentFormat>{}, std::forward<Args>(args)...);

#if FEATURE_EMBR_COAP_OPTION_ECHO
        case n::Echo:
            return f(number_constant<n::Echo>{}, std::forward<Args>(args)...);
#endif

        case n::ETag:
            return f(number_constant<n::ETag>{}, std::forward<Args>(args)...);

        case n::MaxAge:
            return f(number_constant<n::MaxAge>{}, std::forward<Args>(args)...);

        case n::UriHost:
            return f(number_constant<n::UriHost>{}, std::forward<Args>(args)...);

        case n::UriPath:
            return f(number_constant<n::UriPath>{}, std::forward<Args>(args)...);

        case n::UriPort:
            return f(number_constant<n::UriPort>{}, std::forward<Args>(args)...);

        case n::UriQuery:
            return f(number_constant<n::UriQuery>{}, std::forward<Args>(args)...);

        default:
            return no_match(number, std::forward<Args>(args)...);
    }
}

template <class F, class Ret = estd::monostate, class ...Args>
auto dispatch_number(numbers number, Ret no_match, F&& f, Args&&...args)
{
    return dispatch_number_ll(number,
        std::forward<F>(f),
        [&](numbers, Args&&...)
        {
            using retval = decltype(f(number_constant<numbers::Accept>{}, std::forward<Args>(args)...));

            if constexpr(!estd::is_void<retval>::value)
                return no_match;
        });
}

}
