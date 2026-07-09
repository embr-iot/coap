#pragma once

#include "enum.h"

#include <estd/iosfwd.h>

namespace embr::coap::options {

// One-shot flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
uint8_t* delta_length_encode(uint8_t* out, unsigned current_number, numbers number, unsigned length);

template <numbers n>
struct option_traits;

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder;

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder;

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class child_encoder;

}
