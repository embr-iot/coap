#pragma once

#include <embr/net/ethernet.h>

#include <array>
#include <cstdint>
#include <tuple>

namespace devtool::inline core {

struct color
{
    float r, g, b;
};

color mac_to_color(const uint8_t* mac);

inline color mac_to_color(const embr::ethernet::mac& mac)
{
    return mac_to_color(mac.data());
}

}
