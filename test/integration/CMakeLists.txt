cmake_minimum_required(VERSION 3.21)

project(embr-coap-integration-test)

include(../../tools/cmake/setvars.cmake)
include(../../tools/cmake/boost.cmake)

CPMAddPackage("gh:catchorg/Catch2@3.15.1")

add_subdirectory(${ROOT_DIR}/src coap)

set(SOURCE_FILES
    coapme-test.cpp
    coaprd-test.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} Boost::asio embr::coap Catch2WithMain)
