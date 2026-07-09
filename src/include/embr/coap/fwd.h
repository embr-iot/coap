#pragma once

#include <estd/ostream.h>

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder;
    
class header;

struct payload_marker {};

inline namespace markers {

inline constexpr payload_marker payload;

}

}
