#include "devtool/color.h"
#include "devtool/fwd.h"
#include "devtool/lvgl.h"

#include <embr/esp-idf/wifi/fwd.h>

#include <esp_log.h>

#if EMBR_BMGR_LVGL
namespace devtool::inline lvgl {

using namespace embr;

app app::singleton;

[[maybe_unused]]
static const char* TAG = "devtool::lvgl";

lv_color_t mac_to_color(const wifi::mac_type& mac)
{
    const color c = core::mac_to_color(mac);

    //ESP_LOGI(TAG, "mac_to_color: rgb = %f %f %f", c.r, c.g, c.b);

    return lv_color_make(c.r * 255, c.g * 255, c.b * 255);
}

void app::on_button_down()
{
    lv_obj_set_style_bg_color(us_box_, lv_color_make(128, 0, 128), 0);
}

void app::on_button_up()
{
    lv_obj_set_style_bg_color(us_box_, mac_to_color(wifi::get_mac()), 0);
}

void app::on_coap_recv(const embr::wifi::mac_type& source, bool pressed)
{
    if(pressed)
    {
        lv_obj_set_style_bg_color(us_box_, mac_to_color(source), 0);
    }
    else
    {
        lv_obj_set_style_bg_color(us_box_, mac_to_color(wifi::get_mac()), 0);
    }
}

void app::init()
{
    lv_obj_t* screen = lv_screen_active();

    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), 0);
    lv_obj_set_style_text_color(screen, lv_color_white(), 0);

    // "Our" color
    us_box_ = lv_obj_create(screen);
    lv_obj_set_size(us_box_, 100, 50);
    lv_obj_set_style_bg_color(us_box_, mac_to_color(wifi::get_mac()), 0);
    lv_obj_set_align(us_box_, LV_ALIGN_LEFT_MID);

    // "Their" color (incoming MAC)
    them_box_ = lv_obj_create(screen);
    lv_obj_set_size(them_box_, 100, 50);
    lv_obj_set_align(them_box_, LV_ALIGN_RIGHT_MID);

    lv_obj_t* label = lv_label_create(them_box_);
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_label_set_text(label, "Incoming");
    them_label_ = label;
}

}
#endif
