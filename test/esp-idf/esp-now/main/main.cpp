#include <embr/wifi/fwd.h>
#include <embr/esp-idf/net/fwd.h>
#include <embr/esp-idf/wifi/fwd.h>
#include <wifi-console.h>

#include <sdkconfig.h>
#include <esp_board_manager.h>

#include <console_simple_init.h>

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(esp_board_manager_init());

    ESP_ERROR_CHECK(embr::simple_flash_init());

    ESP_ERROR_CHECK(console_cmd_init());

    ESP_ERROR_CHECK(console_cmd_start());
}
