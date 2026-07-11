#pragma once

#include "fwd.h"

#include "../../internal/accumulator.h"

#include <estd/cstdint.h>
#include <estd/optional.h>

namespace embr::coap::options {

class stateful_encoder
{
#if UNIT_TESTING
public:
#endif
    uint16_t current_number_;
    internal::out_accumulator<5> temp_;

    void number_and_length(numbers n, unsigned length);

public:
    stateful_encoder() = default;
    stateful_encoder(estd::nullopt_t) : current_number_{}    {}

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_length(Streambuf& out)  { return temp_.sputn(out); }

    template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
    bool number_and_length(Streambuf& out, numbers n, unsigned length)
    {
        number_and_length(n, length);
        return temp_.sputn(out);
    }
};

}