idf_component_register(
    SRCS button.cpp lvgl.cpp main.cpp
    INCLUDE_DIRS "include"
    REQUIRES
        embr-bmgr
        gen_bmgr_codes esp_wifi
    )
