include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/setvars.cmake)

#list(TRANSFORM SOURCE_FILES PREPEND ${ROOT_DIR}/src/)

idf_component_register(
    REQUIRES embr-coap
    )
