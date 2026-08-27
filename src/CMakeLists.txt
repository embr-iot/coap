cmake_minimum_required(VERSION 3.21)

project(embr-coap-mk2 VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "embr::coap"
    HOMEPAGE_URL "https://github.com/embr-iot/coap"
    )

include(../tools/cmake/setvars.cmake)

CPMAddPackage("gh:malachi-iot/estdlib@0.8.12")

# 23JUL26 MB Not uset yet, bringing in primarily for breadcrumb
#CPMAddPackage("gh:malachi-iot/embr@0.4.7")
CPMAddPackage("gh:malachi-iot/embr#5ebae8e296962df9e9b44a99eab477cb3b06f132")

set(INTERFACE_NAME ${PROJECT_NAME})

include(sources.cmake)

add_library(${INTERFACE_NAME} INTERFACE)
add_library(embr::coap ALIAS ${INTERFACE_NAME})

target_sources(${INTERFACE_NAME} INTERFACE ${SOURCE_FILES} ${INCLUDE_FILES})

target_compile_features(${INTERFACE_NAME} INTERFACE cxx_std_17)

target_link_libraries(${INTERFACE_NAME} INTERFACE estd embr-interface)

target_include_directories(${INTERFACE_NAME} INTERFACE include)
