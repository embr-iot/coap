#include "devtool/fwd.h"
#include "devtool/lvgl.h"

#include <embr/lvgl/indev.h>

#include <esp_log.h>

// 27OCT26 MB DEBT: Was anticipating more code here

#if EMBR_BMGR_LVGL && defined(CONFIG_ESP_BOARD_DEV_LCD_TOUCH_SUPPORT)
static const char* TAG = "devtool::lvgl";

namespace devtool::inline lvgl {

void app::on_touch_pressed(lv_event_t* event)
{
    lv_point_t point = get_point(event).value();
    ESP_LOGD(TAG, "Clicked: %d, %d", point.x, point.y);

    on_button_down();
}

}
#endif