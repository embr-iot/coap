include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/setvars.cmake)

idf_component_register(SRCS
    "lwip-udp.c" pcb.cpp
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi
    )
