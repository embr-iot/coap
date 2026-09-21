#pragma once

#include <sdkconfig.h>

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
#include <embr/bmgr/lvgl.h>

namespace devtool::inline lvgl {

lv_color_t mac_to_color(const uint8_t* mac);

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