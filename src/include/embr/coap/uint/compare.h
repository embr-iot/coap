#pragma once

#include <estd/cstdint.h>
#include <estd/string.h>

// DEBT: Probably ought to put this up into embr proper

namespace embr {

/// Big endian big-unsigned-int compare
/// @param lhs
/// @param lhs_len
/// @param rhs
/// @param rhs_len
/// @return 0 = equal, > 0 = lhs > rhs, < 0 = lhs < rhs
constexpr int be_uintcmp(
    const uint8_t* lhs, unsigned lhs_len,
    const uint8_t* rhs, unsigned rhs_len)
{
    int leading = lhs_len - rhs_len;

    if(leading >= 0)
    {
        while(leading--)   if(*lhs++ != 0)   return 1;

        return memcmp(lhs, rhs, rhs_len);
    }

    while(leading++)   if(*rhs++ != 0)   return -1;

    return memcmp(lhs, rhs, lhs_len);
}

template <unsigned N, unsigned N2>
constexpr int be_uintcmp(const uint8_t (&lhs)[N], const uint8_t (&rhs)[N2])
{
    return be_uintcmp(lhs, N, rhs, N2);
}

}

