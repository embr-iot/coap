#pragma once

#include "fwd.h"

namespace embr::coap::options {

template <numbers n>
using number_constant = estd::integral_constant<numbers, n>;

template <class F, class ...Args>
void dispatch_number(numbers number, F&& f, Args&&...args)
{
    using n = numbers;

    switch(number)
    {
        case n::Accept:
            f(number_constant<n::Accept>{}, std::forward<Args>(args)...);
            break;

        case n::ContentFormat:
            f(number_constant<n::ContentFormat>{}, std::forward<Args>(args)...);
            break;

        case n::UriHost:
            f(number_constant<n::UriHost>{}, std::forward<Args>(args)...);
            break;

        case n::UriPath:
            f(number_constant<n::UriPath>{}, std::forward<Args>(args)...);
            break;

        case n::UriPort:
            f(number_constant<n::UriPort>{}, std::forward<Args>(args)...);
            break;

        case n::UriQuery:
            f(number_constant<n::UriQuery>{}, std::forward<Args>(args)...);
            break;

        default:
            break;
    }
}

}
