#pragma once

#ifdef CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT
#include <embr/bmgr/lvgl.h>

namespace devtool::inline lvgl {

void on_button_down();
void on_button_up();
lv_color_t mac_to_color(const uint8_t* mac);

class screen
{
    lv_obj_t* us_box_{};
    lv_obj_t* them_box_{};

public:
    void init();
};

}
#endif