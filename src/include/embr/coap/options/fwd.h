#pragma once

#include "enum.h"

#include <estd/iosfwd.h>

namespace embr::coap::options {

template <numbers n>
struct option_traits;

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder;

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder;

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class child_encoder;

}
