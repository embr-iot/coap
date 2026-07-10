#pragma once

#include "fwd.h"

#include <estd/span.h>
#include <estd/string_view.h>

namespace embr::coap::options {

struct option_base
{
    unsigned length;

    union
    {
        const uint8_t* opaque_;
        unsigned uint_;
        const char* string_;
    };

    option_base() = default;
    option_base(const option_base&) = default;
};

template <numbers n>
struct option : option_base
{
    static constexpr numbers number = n;

    using traits = option_traits<n>;

    constexpr estd::string_view string() const
    {
        static_assert(traits::format == value_formats::String);

        return { option_base::string_, length };
    }
};


struct option2 : option_base
{
    numbers number;

    // Skirts type-punning issue since technically the last-assigned union member is
    // still active.  Plus, our targets are generally gcc or llvm which type pun anyway
    template <numbers n>
    option2(option<n> convert_from) : number(n),
        option_base(convert_from)
    {
    }
};


}
