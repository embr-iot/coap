#include <embr/wifi/fwd.h>
#include <embr/esp-idf/net/fwd.h>
#include <embr/esp-idf/wifi/fwd.h>
#include <wifi-console.h>

#include <sdkconfig.h>
#include <esp_board_manager.h>

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
#include <embr/bmgr/lvgl.h>
#endif

#include <console_simple_init.h>

void udp_setup();
esp_err_t coap_console_init();

using namespace embr;

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(esp_board_manager_init());

    ESP_ERROR_CHECK(simple_flash_init());

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
    embr_lvgl_init("display_lcd");
#endif

    // Temporarily disabling this so wifi_console behaves better
    //ESP_ERROR_CHECK(wifi::simple_init());

    ESP_ERROR_CHECK(console_cmd_init());

    ESP_ERROR_CHECK(coap_console_init());
    ESP_ERROR_CHECK(wifi_console_init());

    ESP_ERROR_CHECK(console_cmd_start());

    udp_setup();
}
