cmake_minimum_required(VERSION 3.21)

project(embr-coap-mk2 VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "embr::coap"
    HOMEPAGE_URL "https://github.com/embr-iot/coap"
    )

include(../tools/cmake/setvars.cmake)

CPMAddPackage("gh:malachi-iot/estdlib@0.8.12-alpha3")

set(INTERFACE_NAME ${PROJECT_NAME})

include(sources.cmake)

add_library(${INTERFACE_NAME} INTERFACE)
add_library(embr::coap ALIAS ${INTERFACE_NAME})

target_sources(${INTERFACE_NAME} INTERFACE ${SOURCE_FILES} ${INCLUDE_FILES})

target_compile_features(${INTERFACE_NAME} INTERFACE cxx_std_17)

target_link_libraries(${INTERFACE_NAME} INTERFACE estd)

target_include_directories(${INTERFACE_NAME} INTERFACE include)
