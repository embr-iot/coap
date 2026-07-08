#pragma once

namespace embr::coap {

class header;

struct payload_marker {};

inline namespace markers {

inline constexpr payload_marker payload;

}

}
