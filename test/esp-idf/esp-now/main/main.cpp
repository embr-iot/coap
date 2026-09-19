#include <embr/wifi/fwd.h>
#include <embr/esp-idf/net/fwd.h>
#include <embr/esp-idf/wifi/fwd.h>
#include <wifi-console.h>

#include <sdkconfig.h>
#include <esp_board_manager.h>

#include <embr/bmgr/iterator.h>

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
#include <embr/bmgr/lvgl.h>
#endif

#include <console_simple_init.h>

#include <esp_log.h>

namespace {

const char* TAG = "embr::coap: main";

uint8_t mac[6];

lv_color_t mac_to_color(const uint8_t* mac)
{
    unsigned r = mac[0] * mac[1],
        g = mac[2] * mac[3],
        b = mac[4] * mac[5];

    return lv_color_make(r >> 8, g >> 8, b >> 8);
}

lv_color_t mac_to_color() { return mac_to_color(mac); }


}

using namespace embr;

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
void lvgl_setup()
{
    ESP_LOGI(TAG, "lvgl_setup: entry");

    lv_obj_t* screen = lv_screen_active();

    // Not remotely green in appearance
    //lv_obj_set_style_bg_color(screen, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x008000), 0);
    lv_obj_set_style_text_color(screen, lv_color_white(), 0);

    // "Our" color
    lv_obj_t* box = lv_obj_create(screen);
    lv_obj_set_size(box, 100, 50);
    lv_obj_set_align(box, LV_ALIGN_CENTER);
    // FIX: Always appears black
    lv_obj_set_style_bg_color(box, mac_to_color(), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);

    lv_obj_t* label = lv_label_create(screen);
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_label_set_text(label, "Hello world");
}
#endif

#ifdef CONFIG_ESP_BOARD_DEV_BUTTON_SUPPORT
#include <embr/bmgr/dev_button.h>

esp_err_t button_init()
{
    bmgr::device_name_iterator<bmgr::constants::device_type_button> it;

    // FIX: Need auto-bump in the start
    ++it;

    bmgr::dev_button b;

    // FIX: iterator presents operator wrong
    ESP_RETURN_ON_ERROR(b.get(it), TAG, "Cannot acquire first button");

    return ESP_OK;
}
#endif

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(simple_flash_init());

    ESP_ERROR_CHECK(esp_board_manager_init());

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
    ESP_ERROR_CHECK_WITHOUT_ABORT(embr_lvgl_init());

    lvgl::async_call([] { lvgl_setup(); });
#endif
#ifdef CONFIG_ESP_BOARD_DEV_BUTTON_SUPPORT
    ESP_ERROR_CHECK(button_init());
#endif

    ESP_ERROR_CHECK(wifi::preinit());
    ESP_ERROR_CHECK(wifi::esp_now_init());
    ESP_ERROR_CHECK(wifi::esp_now_start(1));

    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));

    ESP_ERROR_CHECK(console_cmd_init());

    ESP_ERROR_CHECK(console_cmd_start());
}
