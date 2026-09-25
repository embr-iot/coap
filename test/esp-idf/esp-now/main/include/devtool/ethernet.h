#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

// 25SEP26 DEBT: Experimental namespacing.  Also, I think embr::net ought to make
// it embr::net and not embr::inline net
namespace embr::inline net::inline ethernet {

using mac_type = std::array<uint8_t, 6>;

constexpr mac_type make_mac(const uint8_t* copy_from)
{
    mac_type mac;
    std::copy_n(copy_from, 6, mac.begin());
    return mac;
}

}
