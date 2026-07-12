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

        return { string_, length };
    }

    constexpr estd::span<const uint8_t> opaque() const
    {
        static_assert(traits::format == value_formats::Opaque);

        return { opaque_, length };
    }

    constexpr unsigned uint() const
    {
        static_assert(traits::format == value_formats::Uint);

        return uint_;

    }
};


struct option2 : option_base
{
    numbers number;

    // only used by stateful decoder
    bool end;

    option2() = default;

    // Skirts type-punning issue since technically the last-assigned union member is
    // still active.  Plus, our targets are generally gcc or llvm which type pun anyway
    template <numbers n>
    option2(option<n> convert_from) : number(n),
        option_base(convert_from)
    {
    }
};

//template <numbers n>
//constexpr bool is_constexpr(option<n>) { return true; }
//constexpr bool is_constexpr(option2) { return false; }

template <class T>
struct is_constexpr_t;

template <>
struct is_constexpr_t<option2> : estd::false_type {};

template <numbers n>
struct is_constexpr_t<option<n>> : estd::true_type {};

template <class T>
constexpr bool is_constexpr = is_constexpr_t<estd::remove_cvref_t<T>>::value;

static_assert(!is_constexpr<option2>);
static_assert(is_constexpr<option<numbers::Accept>>);

}
