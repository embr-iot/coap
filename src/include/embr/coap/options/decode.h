#pragma once

#include <estd/cstdint.h>

#include "fwd.h"

namespace embr::coap::options {

// One-shot flavor
const uint8_t* delta_length_decode(const uint8_t* in, unsigned number_current, numbers* number, unsigned* length);

// State machine flavor
// https://datatracker.ietf.org/doc/html/rfc7252#section-3.1
class delta_length_decoder
{

};

}
