#pragma once

#include <estd/span.h>
#include <estd/ostream.h>

struct restrained_ospanbuf : estd::ospanbuf
{
    struct policy
    {
        // EXPERIMENTAL
        // See https://github.com/malachi-iot/estdlib/issues/219
        static constexpr bool one_shot = false;
    };

    using base_type = estd::ospanbuf;

    unsigned limiter = 10000;

    template <class ...Args>
    restrained_ospanbuf(Args&&...args) : base_type(std::forward<Args>(args)...)    {}

    int sputn(const char_type* data, unsigned sz)
    {
        return base_type::sputn(data, std::min(limiter, sz));
    }
};
