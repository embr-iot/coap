#include "devtool/lvgl.h"

#include <embr/wifi/fwd.h>
#include <embr/esp-idf/net/fwd.h>
#include <embr/esp-idf/wifi/fwd.h>
#include <wifi-console.h>

#include <sdkconfig.h>
#include <esp_board_manager.h>

#include <console_simple_init.h>

#include <esp_log.h>

namespace {

const char* TAG = "embr::coap: main";

}

using namespace embr;

esp_err_t button_init();

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(simple_flash_init());

    ESP_ERROR_CHECK(esp_board_manager_init());

    ESP_ERROR_CHECK(wifi::preinit());
    ESP_ERROR_CHECK(wifi::esp_now_init());
    ESP_ERROR_CHECK(wifi::esp_now_start(1));

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
    ESP_ERROR_CHECK_WITHOUT_ABORT(embr_lvgl_init());

    lvgl::async_call([] { devtool::lvgl::app::singleton.init(); });
#endif
    ESP_ERROR_CHECK(button_init());

    ESP_ERROR_CHECK(console_cmd_init());

    ESP_ERROR_CHECK(console_cmd_start());
}
