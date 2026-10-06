#pragma once

#include <estd/type_traits.h>

namespace embr::coap::internal {

// 06OCT26 DEBT: Refactor to be rdbuf_ not out_ so we can reuse this elsewhere
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
    using int_type = typename streambuf_type::int_type;
    using pos_type = typename streambuf_type::pos_type;
    using char_type = typename streambuf_type::char_type;
    using const_pointer = const char_type*;

    using streambuf_policy = typename streambuf_type::policy;

    // DEBT: Perhaps we want in_ flavors supported too?

    streambuf_type& out() { return out_; }
    const Streambuf& out() const { return out_; }

    // EXPERIMENTAL
    streambuf_type& rdbuf() { return out_; }
    const streambuf_type& rdbuf() const { return out_; }
};

}   // namespace embr::coap::internal
