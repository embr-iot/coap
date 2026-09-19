idf_component_register(
    SRCS main.cpp
    INCLUDE_DIRS "."
    REQUIRES
        embr-bmgr
        gen_bmgr_codes esp_wifi
    )
