#include "devtool/lvgl.h"

#include <embr/coap/encoder.h>
#include <embr/coap/options/encode.h>

#include <embr/bmgr/dev_button.h>
#include <embr/bmgr/iterator.h>

#include <esp_log.h>

using namespace embr;

namespace {

int send_counter = 0;

const char* TAG = "embr::coap: button";

void callback(void* arg, void* usr_data)
{
    char out[32];
    using encoder_type = coap::encoder<estd::ospanbuf>;
    encoder_type encoder(out);
    
    auto button = static_cast<button_handle_t>(arg);

    auto name = static_cast<const char*>(usr_data);

    button_event_t event = iot_button_get_event(button);

    ESP_LOGV(TAG, "callback: %s", name);

    switch(event)
    {
        case BUTTON_PRESS_DOWN:
            using namespace embr::coap;

            encoder << header(header::NON, header::PUT);
            encoder << payload << ++send_counter;

            // TODO: Actually send (broadcast) the packet

#if EMBR_BMGR_LVGL
            lvgl::async_call([] { devtool::app::singleton.on_button_down(); });
#endif
            break;

        case BUTTON_PRESS_UP:
#if EMBR_BMGR_LVGL
            lvgl::async_call([] { devtool::app::singleton.on_button_up(); });
#endif
            break;

        default:
            abort();
    }
}

}

esp_err_t button_init()
{
    if constexpr(bmgr::dev_button::supported)
    {
        bmgr::device_name_iterator<bmgr::constants::device_type_button> it;

        bmgr::dev_button b;

        ESP_RETURN_ON_ERROR(b.get(*it), TAG, "Cannot acquire first button");
        ESP_RETURN_ON_ERROR(
            b.register_cb(BUTTON_PRESS_DOWN, callback, (void*)*it),
            TAG, "Cannot register callback");
        ESP_RETURN_ON_ERROR(
            b.register_cb(BUTTON_PRESS_UP, callback, (void*)*it),
            TAG, "Cannot register callback");
    }

    return ESP_OK;
}

