#pragma once

#include <embr/bmgr/dev_led_strip.h>

#include <embr/esp-idf/wifi/fwd.h>

#include <esp_wifi.h>

// DEBT: Named this folder 'devtool' out of habit, but that's inaccurate for this ESP-NOW
// specific test

// DEBT: Consider embr::inline esp_idf::wifi - however that may collide with legacy
// embr things.  Perhaps put it all into embr::esp_idf::wifi then alias it in?
// experimenting with that
// DEBT: Put this into embr::net

namespace embr::wifi {

// 26SEP26 DEBT: Kinda crude
constexpr const uint8_t* broadcast_mac = ethernet::addr::broadcast.data();

using namespace embr::esp_idf::wifi;

}

namespace devtool::inline core {

struct color;

extern embr::bmgr::dev_led_strip led_strip;

// DEBT: Make this configurable.  These dudes can be bright!
static constexpr float led_intensity = 0.2;

esp_err_t set_pixel(const color&);

}

inline namespace test {

// 26SEP26 DEBT: Part of debt where we misused 'devtool' as described above
using namespace devtool::core;

}

