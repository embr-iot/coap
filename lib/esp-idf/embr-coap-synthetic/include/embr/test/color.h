#pragma once

#include <embr/net/ethernet.h>

#include <array>
#include <cstdint>
#include <tuple>

namespace embr::inline test {

struct color
{
    float r, g, b;
};

namespace colors {

constexpr color black   {0, 0, 0};
constexpr color lime    {0, 1, 0};
constexpr color green   (0, 0.5, 0);
}

color mac_to_color(const uint8_t* mac);

inline color mac_to_color(const embr::ethernet::mac& mac)
{
    return mac_to_color(mac.data());
}

}
