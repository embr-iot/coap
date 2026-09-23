#include "devtool/fwd.h"
#include "devtool/lvgl.h"

#include <embr/wifi/fwd.h>
#include <embr/esp-idf/net/fwd.h>
#include <embr/esp-idf/wifi/fwd.h>
#include <wifi-console.h>

#include <sdkconfig.h>
#include <esp_board_manager.h>

#include <console_simple_init.h>

#include <esp_log.h>
#include <esp_now.h>

namespace {

const char* TAG = "embr::coap: main";

}

using namespace embr;

esp_err_t button_init();
void rgb_init();

// DEBT: Put some of this into embr::net
static void _esp_now_init()
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb([](
        const esp_now_recv_info_t* esp_now_info,
        const uint8_t* data, int data_len)
    {

    }));

    esp_now_peer_info_t peer{};
    //peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    memcpy(peer.peer_addr, wifi::broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
}

extern "C" void app_main(void)
{
    esp_board_manager_print_board_info();

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
    _esp_now_init();
    rgb_init();

    ESP_ERROR_CHECK(console_cmd_init());

    ESP_ERROR_CHECK(console_cmd_start());
}
