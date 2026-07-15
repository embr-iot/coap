idf_component_register(
    SRCS flash.cpp wifi.cpp
    INCLUDE_DIRS "include"
    REQUIRES estd embr esp_wifi nvs_flash
    )
