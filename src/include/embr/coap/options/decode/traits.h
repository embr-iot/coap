#pragma once

#include "../../internal/policies.h"

namespace embr::coap::options {

struct decoder_traits_base
{
    using policies = internal::policies_enum::policies;
};

struct presumptive_decoder_traits : decoder_traits_base
{
    static constexpr policies policy = policies::Presumptive;
};

struct retry_decoder_traits : decoder_traits_base
{
    static constexpr policies policy = policies::Retry;

    // IDEA: Can also be 0, indicating consumer must manually bump stream forward
    // themselves.  Interesting also might be ext mode for in_accumulator, but that's
    // less obvious how to do cleanly
    static constexpr unsigned in_accumulator_size = 64;
};

}
