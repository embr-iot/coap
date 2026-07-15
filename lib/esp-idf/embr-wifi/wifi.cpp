#include "embr/wifi/fwd.h"

#include <esp_log.h>
#include <esp_wifi.h>

#include <cstring>

static const char* TAG = "embr::wifi";

namespace embr {

static void event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    ESP_LOGV(TAG, "event_base=%s, event_id=%ld", event_base, event_id);

    if(event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            default:
                break;
        }
    }
}

esp_err_t simple_wifi_init(esp_netif_t** wifi_netif)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t* esp_netif = esp_netif_create_default_wifi_sta();

    if(wifi_netif)  *wifi_netif = esp_netif;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config{};

    strcpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD);

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    return esp_wifi_start();
}

}
