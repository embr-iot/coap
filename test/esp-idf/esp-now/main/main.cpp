#include "devtool/color.h"
#include "devtool/fwd.h"
#include "devtool/lvgl.h"

#include <embr/coap/decoder.h>

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
        using namespace coap;
        using decoder_type = decoder<estd::detail::basic_ispanbuf<const uint8_t>>;

        decoder_type decoder(data, data_len);

        header h;
        token t;
        options::option opt;

        decoder >> h >> t;
        decoder >> opt;

        assert(h.type() == header::NON);
        assert(h.code() == header::PUT);

        if(decoder.state() != decoder_type::Payload)
        {
            ESP_LOGW(TAG, "decoder.state() = %s", to_string(decoder.state()));
            return;
        }

        int c = decoder.in().sbumpc();

        ESP_LOGI(TAG, "c = %c", c);

        bool pressed;

        switch(c)
        {
            case '0':
                pressed = false;
                break;

            case '1':
                pressed = true;
                break;

            default:
                pressed = false;    // FIX: Annoying, compiler warning -> err demands this
                ESP_LOGW(TAG, "Unknown pressed state, results may vary");
                break;
        }

#ifdef CONFIG_ESP_BOARD_DEV_LED_STRIP_SUPPORT
        //if constexpr(devtool::led_strip.supported)
        {
            using namespace devtool;
        
            const color c = pressed ?
                core::mac_to_color(esp_now_info->src_addr) :
                core::mac_to_color(wifi::get_mac());

            ESP_ERROR_CHECK(set_pixel(c));
            ESP_ERROR_CHECK(led_strip.refresh());
        }
#endif

#if EMBR_BMGR_LVGL
        ethernet::mac src_addr = wifi::make_mac(esp_now_info->src_addr);
        lvgl::async_call([src_addr, pressed]
            {
                devtool::lvgl::app::singleton.on_coap_recv(src_addr, pressed);
            });
#endif
    }));

    esp_now_peer_info_t peer{};
    //peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    memcpy(peer.peer_addr, ethernet::addr::broadcast.data(), ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
}

extern "C" void app_main(void)
{
    esp_board_manager_print_board_info();

    ESP_ERROR_CHECK(esp_idf::simple_flash_init());

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
