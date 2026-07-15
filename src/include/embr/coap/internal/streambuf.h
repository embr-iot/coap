#pragma once

#include <estd/type_traits.h>

namespace embr::coap::internal {

template <class Streambuf>
class streambuf_provider
{
protected:
    Streambuf out_;

public:
    template <class ...Args>
    constexpr explicit streambuf_provider(Args&&... args) :
        out_(std::forward<Args>(args)...)
    {}

    using streambuf_type = estd::remove_cvref_t<Streambuf>;
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    // DEBT: Perhaps we want in_ flavors supported too?

    streambuf_type& out() { return out_; }
    const Streambuf& out() const { return out_; }
};

}
