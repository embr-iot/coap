#pragma once

#include "traits.h"
#include "../enum.h"

#include <estd/cstdint.h>
#include <estd/streambuf.h>

namespace embr::coap::options {

// One-shot flavor
const uint8_t* delta_length_decode(const uint8_t* in, unsigned number_current, numbers* number, unsigned* length);

// Decode one option only
// Low-level because caller must manually move  stream forward
// As name implies, not stateful
template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::optional<int> delta_length_decode(Streambuf& in, F&&);

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits = presumptive_decoder_traits>
class decoder;

class stateful_decoder;

}
