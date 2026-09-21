#include "devtool/color.h"
#include "devtool/lvgl.h"

#if EMBR_BMGR_LVGL
namespace devtool::inline lvgl {

lv_color_t mac_to_color(const uint8_t* mac)
{
    color c = devtool::mac_to_color;

    return lv_color_make(c.r * 255, c.g * 255, c.b * 255);
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
#endif
