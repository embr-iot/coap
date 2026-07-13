cmake_minimum_required(VERSION 3.16)

if(ESP_PLATFORM)
    include(${CMAKE_CURRENT_LIST_DIR}/tools/esp-idf/embr-coap/CMakeLists.txt)
else()
    set(EMBR_COAP_BUILD_TESTS OFF CACHE BOOL "Build embr tests")

    project(embr-coap-meta)

    if(EMBR_COAP_BUILD_TESTS OR PROJECT_IS_TOP_LEVEL)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test/catch catch)
    else()
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src embr-coap)
    endif()
endif()
