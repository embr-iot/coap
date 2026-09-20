#include "devtool/lvgl.h"

namespace devtool::inline lvgl {

lv_color_t mac_to_color(const uint8_t* mac)
{
    unsigned r = mac[0] * mac[1],
        g = mac[2] * mac[3],
        b = mac[4] * mac[5];

    return lv_color_make(r >> 8, g >> 8, b >> 8);
}

void on_button_down()
{

}

void on_button_up()
{

}

void screen::init()
{
    lv_obj_t* screen = lv_screen_active();

    // "Our" color
    us_box_ = lv_obj_create(screen);

    // "Their" color (incoming MAC)
    them_box_ = lv_obj_create(screen);
}

}
