#include "devtool/color.h"
#include "devtool/fwd.h"

#include <embr/bmgr/dev_button.h>
#include <embr/bmgr/dev_led_strip.h>
#include <embr/bmgr/iterator.h>

#include <esp_log.h>

using namespace embr;

static const char* TAG = "devtool::misc";

namespace devtool::inline core {

color mac_to_color(const uint8_t* mac)
{
    float r = mac[0] * mac[1],
        g = mac[2] * mac[3],
        b = mac[4] * mac[5];

    return { r / 0xFFFF, g / 0xFFFF, b / 0xFFFF };
}

embr::bmgr::dev_led_strip led_strip;

#ifdef CONFIG_ESP_BOARD_DEV_LED_STRIP_SUPPORT
esp_err_t set_pixel(const color& c)
{
    return led_strip.set_pixel(0,
        c.r * led_intensity * 255,
        c.g * led_intensity * 255,
        c.b * led_intensity * 255);
}
#endif


}

void rgb_init()
{
    // 26SEP26 DEBT: I guess if constexpr doesn't help here as much as I thought.
    // reminds me of SFINAE type of things.
#ifdef CONFIG_ESP_BOARD_DEV_LED_STRIP_SUPPORT
    static_assert(bmgr::dev_led_strip::supported);

    if constexpr(bmgr::dev_led_strip::supported)
    {
        ESP_LOGI(TAG, "rgb_init: entry");

        using led_strip_iterator = bmgr::device_iterator<bmgr::constants::device_type_led_strip>;

        led_strip_iterator it = led_strip_iterator::begin();

        assert(it->is_valid());

        devtool::led_strip = *it;

        const devtool::color c = devtool::mac_to_color(wifi::get_mac());

        ESP_LOGI(TAG, "mac_to_color: rgb = %f %f %f", c.r, c.g, c.b);

        ESP_ERROR_CHECK(set_pixel(c));
        ESP_ERROR_CHECK(it->refresh());
    }
#endif
}
