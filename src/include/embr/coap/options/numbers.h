#pragma once

#include "fwd.h"

namespace embr::coap::options {

template <numbers n>
using number_constant = estd::integral_constant<numbers, n>;

template <class F, class Ret = estd::monostate, class ...Args>
auto dispatch_number(numbers number, Ret no_match, F&& f, Args&&...args)
{
    using n = numbers;
    using retval = decltype(f(number_constant<n::Accept>{}, std::forward<Args>(args)...));

    switch(number)
    {
        case n::Accept:
            return f(number_constant<n::Accept>{}, std::forward<Args>(args)...);

        case n::Block1:
            return f(number_constant<n::Block1>{}, std::forward<Args>(args)...);

        case n::Block2:
            return f(number_constant<n::Block2>{}, std::forward<Args>(args)...);

        case n::ContentFormat:
            return f(number_constant<n::ContentFormat>{}, std::forward<Args>(args)...);

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
            if constexpr(!estd::is_void<retval>::value)
            {
                return no_match;
            }
            break;
    }
}

}
