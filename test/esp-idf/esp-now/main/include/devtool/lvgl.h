#pragma once

#include <sdkconfig.h>

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
#include "fwd.h"

#include <embr/bmgr/lvgl.h>

#include <array>

namespace devtool::inline lvgl {

lv_color_t mac_to_color(const embr::wifi::mac_type&);

class app
{
    lv_obj_t* us_box_{};
    lv_obj_t* them_box_{};
    lv_obj_t* them_label_{};

public:
    static app singleton;

    void init();
    void on_button_down();
    void on_button_up();
};

}
#endif