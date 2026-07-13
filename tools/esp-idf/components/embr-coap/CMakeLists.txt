#set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)
include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/setvars.cmake)
include(${ROOT_DIR}/src/sources.cmake)

list(TRANSFORM SOURCE_FILES PREPEND ${ROOT_DIR}/src/)

idf_component_register(
    SRCS ${SOURCE_FILES}
    INCLUDE_DIRS ${ROOT_DIR}/src/include)
