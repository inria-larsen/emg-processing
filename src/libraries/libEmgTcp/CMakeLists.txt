# Copyright: (C) 2017 Inria
# Author: Waldez Gomes
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(emgtcp)

include_directories(${YARP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include/EmgTcp include)

set(LIB_TYPE SHARED)

add_library(EmgTcp SHARED src/EmgTcp.cpp)

install(TARGETS EmgTcp DESTINATION lib)

install(DIRECTORY "include/" DESTINATION include)


