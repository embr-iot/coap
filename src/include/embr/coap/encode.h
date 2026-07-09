#pragma once

#include "options/encode.h"

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder
{
    Streambuf out_;
    options::encoder<Streambuf&> options_{out_};

    //using payload_type = options::payload_encoder<Streambuf&>;
    using payload_type = estd::detail::basic_ostream<Streambuf&>;

public:
    template <class ...Args>
    constexpr explicit encoder(Args&&... args) : out_(std::forward<Args>(args)...) {}

    template <options::numbers n>
    options::single_encoder<n, Streambuf&> operator<<(options::option_marker<n>)
    {
        return { &options_ };
    }

    payload_type& operator<<(payload_marker)
    {
        return options_ << payload_marker{};
    }
};

}
