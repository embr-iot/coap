#pragma once

#include <sdkconfig.h>

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
#include "fwd.h"

#include <embr/bmgr/lvgl.h>
#include <embr/lvgl/obj.h>

// DEBT: Consider doing this up in embr/lvgl/fwd.h as a Kconfig option,
// though maybe better to do this one-liner for when we really
// want it
using namespace embr::lvgl::wrapper;

namespace devtool::inline lvgl {

lv_color_t mac_to_color(const embr::ethernet::mac&);

class app
{
    object us_box_{};
    lv_obj_t* them_box_{};
    lv_obj_t* them_label_{};

    void on_touch_pressed(lv_event_t*);

public:
    static app singleton;

    void init();
    void on_button_down();
    void on_button_up();
    void on_coap_recv(const embr::ethernet::mac&, bool pressed);
};

}
#endif