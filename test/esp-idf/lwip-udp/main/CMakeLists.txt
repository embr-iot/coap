idf_component_register(SRCS
    console.cpp "main.cpp" pcb.cpp
    INCLUDE_DIRS "include"
    REQUIRES embr-net esp_wifi
    )
