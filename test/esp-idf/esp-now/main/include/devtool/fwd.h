#pragma once

#include <embr/bmgr/fwd.h>  // DEBT: dev_led_strip should include this themself
#include <embr/bmgr/dev_led_strip.h>

#include <esp_wifi.h>

#include <array>

// DEBT: Consider embr::inline esp_idf::wifi - however that may collide with legacy
// embr things.  Perhaps put it all into embr::esp_idf::wifi then alias it in?
// experimenting with that
// DEBT: Put this into embr::net

namespace embr::esp_idf::wifi {

inline std::array<uint8_t, 6> get_mac(wifi_interface_t interface = WIFI_IF_STA)
{
    std::array<uint8_t, 6> mac;

    ESP_ERROR_CHECK(esp_wifi_get_mac(interface, mac.data()));

    return mac; // RVO we're relying on you buddy
}

}

namespace embr::wifi {

constexpr uint8_t broadcast_mac[] { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

using namespace embr::esp_idf::wifi;

}

namespace devtool::inline core {

extern embr::bmgr::dev_led_strip led_strip;

}

