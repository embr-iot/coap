idf_component_register(
    SRCS flash.cpp
    INCLUDE_DIRS "include"
    REQUIRES estd embr embr-net esp_wifi nvs_flash
    )
