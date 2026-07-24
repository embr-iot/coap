include_guard()

set(BOOST_VERSION "1.91.0-1")

set(BOOST_ENABLE_CMAKE ON CACHE BOOL "")
set(BOOST_INCLUDE_LIBRARIES asio;signals2 CACHE STRING "")
set(BOOST_ENABLE_TESTING OFF CACHE BOOL "")

include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

CPMAddPackage(
    NAME boost
    VERSION ${BOOST_VERSION}
    URL https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}-cmake.tar.gz
)
