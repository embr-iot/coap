include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/setvars.cmake)

idf_component_register(SRCS "lwip-udp.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi
    )
