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

    static constexpr unsigned in_accumulator_size = 32;
};

}
