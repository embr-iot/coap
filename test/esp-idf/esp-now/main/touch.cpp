#include "devtool/fwd.h"
#include "devtool/lvgl.h"

#include <esp_log.h>

// 27OCT26 MB DEBT: Was anticipating more code here

#if EMBR_BMGR_LVGL && defined(CONFIG_ESP_BOARD_DEV_LCD_TOUCH_SUPPORT)
static const char* TAG = "devtool::lvgl";

namespace devtool::inline lvgl {

std::optional<lv_point_t> get_point(lv_event_t* event)
{
    lv_indev_t* indev = lv_event_get_indev(event);
    if(indev == nullptr)    return std::nullopt;
    lv_point_t point;
    lv_indev_get_point(indev, &point);
    return point;
}

void app::on_touch_pressed(lv_event_t* event)
{
    lv_point_t point = get_point(event).value();
    ESP_LOGD(TAG, "Clicked: %d, %d", point.x, point.y);

    on_button_down();
}

void app::on_touch_released(lv_event_t*)
{
    on_button_up();
}

}
#endif