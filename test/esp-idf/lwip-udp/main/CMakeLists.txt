idf_component_register(SRCS
    console.cpp lvgl.cpp "main.cpp" pcb.cpp
    INCLUDE_DIRS "include"
    REQUIRES gen_bmgr_codes embr-net esp_wifi
    )
