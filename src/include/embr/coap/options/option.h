#pragma once

#include "fwd.h"

#include "../uint.h"

#include <estd/span.h>
#include <estd/string_view.h>

namespace embr::coap::options {

struct option_base
{
    unsigned length;

    union
    {
        const uint8_t* opaque_;
        // DEBT: it's easily possible for an int bigger than an unsigned to appear
        unsigned uint_;
        const char* string_;
        const void* raw_;
    };

    option_base() = default;

    constexpr option_base(const option_base&) = default;
    constexpr option_base(unsigned length, const void* raw) :
        length{length},
        raw_{raw}
    {}
};

template <numbers n = {}>
struct option : option_base
{
    static constexpr numbers number = n;

    using traits = option_traits<n>;

    [[nodiscard]] constexpr estd::string_view string() const
    {
        static_assert(traits::format == value_formats::String);

        return { string_, length };
    }

    [[nodiscard]] constexpr estd::span<const uint8_t> opaque() const
    {
        static_assert(traits::format == value_formats::Opaque);

        return { opaque_, length };
    }

    template <class Integer = unsigned>
    [[nodiscard]] constexpr Integer uint() const
    {
        static_assert(traits::format == value_formats::Uint);

        // DEBT: Switch over and decode to a bigger int type if we need to/can
        //if(length > sizeof(uint_)) {}

        return uint_;
    }

    static constexpr bool is_assignable = traits::format == value_formats::Opaque ||
        traits::format == value_formats::String ||
        traits::format == value_formats::Uint;

    void value(const uint8_t* v)
    {
        if constexpr(traits::format == value_formats::Opaque)
        {
            opaque_ = v;
        }
        else if constexpr(traits::format == value_formats::String)
        {
            string_ = (const char*)v;
        }
        else if constexpr(traits::format == value_formats::Uint)
        {
            uint_ = uint_decode<unsigned>(v, length);
        }

        static_assert(is_assignable, "value not assignable for this option");
    }
};

// no constexpr/traits for u
template <>
struct option<numbers{}> : private option_base
{
    // non-specialized option doesn't know whether he's opaque, string, etc. and
    // producer also doesn't (otherwise we could dispatch and specialize).
    // Therefore, all values are presented as opaque_
    using option_base::length;
    using option_base::opaque_;

    numbers number;

    // only used by stateful decoder. represents whether this option
    // is a partial chunk (end == false) or the buffer goes all the way
    // to the end
    bool end;

    option() = default;

    // Skirts type-punning issue since technically the last-assigned union member is
    // still active.  Plus, our targets are generally gcc or llvm which type pun anyway
    template <numbers n>
    constexpr option(const option<n>& convert_from) : number(n),
        option_base(convert_from)
    {
    }

    option(numbers number, unsigned length, const void* raw) :
        option_base(length, raw),
        number{number}
    {}

    constexpr estd::string_view string() const
    {
        return { string_, length };
    }

    constexpr estd::span<const uint8_t> opaque() const
    {
        return { opaque_, length };
    }

    constexpr unsigned uint() const
    {
        return uint_decode(opaque_, length);
    }


    template <class F, class ...Args>
    bool dispatch(F&& f, Args&&...args)
    {
        return dispatch_number(number, [&](const auto number)
        {
            option<number> o;
            o.length = length;
            o.value(opaque_);
            f(o, std::forward<Args>(args)...);
            return true;
        }, false);
    }
};

//template <numbers n>
//constexpr bool is_constexpr(option<n>) { return true; }
//constexpr bool is_constexpr(option2) { return false; }

template <class T>
struct is_constexpr_t;

template <>
struct is_constexpr_t<option<>> : estd::false_type {};

template <numbers n>
struct is_constexpr_t<option<n>> : estd::true_type {};

template <class T>
constexpr bool is_constexpr = is_constexpr_t<estd::remove_cvref_t<T>>::value;

static_assert(!is_constexpr<option<>>);
static_assert(is_constexpr<option<numbers::Accept>>);

}
