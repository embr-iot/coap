#pragma once

#include "fwd.h"

namespace embr::coap::options {

// One-shot flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
uint8_t* delta_length_encode(uint8_t* out, unsigned current_number, numbers number, unsigned length);

}
