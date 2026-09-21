#include "devtool/color.h"

namespace devtool {

color mac_to_color(const uint8_t* mac)
{
    float r = mac[0] * mac[1],
        g = mac[2] * mac[3],
        b = mac[4] * mac[5];

    return { r / 0xFFFF, g / 0xFFFF, b / 0xFFFF };
}

}
