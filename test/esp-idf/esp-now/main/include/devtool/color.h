#pragma once

#include <cstdint>
#include <tuple>

namespace devtool::inline core {

struct color
{
    float r, g, b;
};

color mac_to_color(const uint8_t* mac);

}
