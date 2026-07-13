include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/setvars.cmake)
include(${ROOT_DIR}/test/catch/sources.cmake)

list(TRANSFORM SOURCE_FILES PREPEND ${ROOT_DIR}/test/catch/)

idf_component_register(SRCS
     ${SOURCE_FILES}
     "test_main.cpp"
     INCLUDE_DIRS "."
     WHOLE_ARCHIVE)

target_compile_options(${COMPONENT_LIB} PUBLIC -Wno-unused-variable)
target_compile_definitions(${COMPONENT_LIB} PUBLIC UNIT_TESTING=1)
