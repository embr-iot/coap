#pragma once

#include <array>
#include <cstdint>
#include <tuple>

namespace devtool::inline core {

struct color
{
    float r, g, b;
};

color mac_to_color(const uint8_t* mac);

inline color mac_to_color(const std::array<uint8_t, 6> mac)
{
    return mac_to_color(mac.data());
}

}
