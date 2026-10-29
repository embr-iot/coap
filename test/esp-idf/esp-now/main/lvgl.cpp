#include "devtool/fwd.h"
#include "devtool/lvgl.h"

#include <embr/esp-idf/wifi/fwd.h>
#include <embr/lvgl/obj_event.h>

#ifdef CONFIG_ESP_BOARD_DEV_LCD_TOUCH_SUPPORT
#include <embr/bmgr/dev/lcd_touch.h>
#include <embr/bmgr/iterator.h>
#endif
#include <embr/test/color.h>

#include <estd/sstream.h>
#include <estd/iomanip.h>   // FIX: Can't place above due to incomplete dependencies

#include <esp_log.h>

#if EMBR_BMGR_LVGL
namespace devtool::inline lvgl {

using namespace embr;

app app::singleton;

[[maybe_unused]]
static const char* TAG = "devtool::lvgl";

lv_color_t mac_to_color(const ethernet::mac& mac)
{
    const color c = test::mac_to_color(mac);

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

void app::on_coap_recv(const ethernet::mac& source, bool pressed)
{
    using namespace estd;

    if(pressed)
    {
        estd::layer1::ostringstream<32> out;

        // DEBT: See https://github.com/embr-iot/net/issues/1
        out << "MAC:\n" << hex << setw(2);
        out << source[0] << ':';
        for(int i = 1; i < 6; ++i)  out << ':' << source[i];

        lv_obj_set_style_bg_color(us_box_, mac_to_color(source), 0);

        lv_label_set_text(them_label_, out.rdbuf()->str().c_str());
    }
    else
    {
        lv_obj_set_style_bg_color(us_box_, mac_to_color(wifi::get_mac()), 0);

        lv_label_set_text(them_label_, "Incoming");
    }
}


void app::init()
{
    object screen = lv_screen_active();

#ifdef CONFIG_EMBR_LVGL_INIT_TOUCH
    add_event_cb<&app::on_touch_pressed>(screen, LV_EVENT_PRESSED, this);
    add_event_cb(screen, 
        [](lv_event_t*, app* self) { self->on_button_up(); },
        LV_EVENT_RELEASED, this);
#endif

    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), 0);
    lv_obj_set_style_text_color(screen, lv_color_white(), 0);

    constexpr bool small_mode = true;

    // "Our" color
    us_box_ = screen.create();
    us_box_.set_size(100, 50);
    lv_obj_set_style_bg_color(us_box_, mac_to_color(wifi::get_mac()), 0);
    lv_obj_set_align(us_box_, LV_ALIGN_LEFT_MID);

    // "Their" color (incoming MAC)
    them_box_ = screen.create();
    lv_obj_set_size(them_box_, 100, 50);
    lv_obj_set_align(them_box_, LV_ALIGN_RIGHT_MID);

    lv_obj_t* label = lv_label_create(them_box_);
    lv_obj_set_align(label, LV_ALIGN_CENTER);

    if(small_mode)
        lv_obj_set_style_text_font(label, &lv_font_montserrat_10, LV_PART_MAIN);

    them_label_ = label;
    on_coap_recv({}, false);    // Dogfood our "Incoming"
}

}
#endif
