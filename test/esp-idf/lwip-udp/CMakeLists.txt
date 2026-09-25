cmake_minimum_required(VERSION 3.22)

set(COMPONENTS main)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 25SEP26 - bmgr is pissed right off about this guy.  I guess he inspects looking for
# project.cmake
#include(../../../tools/esp-idf/setenv.cmake)

project(lwip-udp)
