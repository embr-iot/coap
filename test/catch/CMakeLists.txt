cmake_minimum_required(VERSION 3.21)

project(embr-coap-test)

add_definitions(-DUNIT_TESTING)

include(../../tools/cmake/setvars.cmake)

CPMAddPackage("gh:catchorg/Catch2@3.15.1")

add_subdirectory(${ROOT_DIR}/src coap)

set(SOURCE_FILES
    decode-test.cpp
    encode-test.cpp
    header-test.cpp
    options-decode-test.cpp
    options-encode-test.cpp
    uint-test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} embr::coap Catch2WithMain)

target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING=1)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Makes concepts-related failures easier to read
    target_compile_options(${PROJECT_NAME} PUBLIC -fconcepts-diagnostics-depth=2)
endif()