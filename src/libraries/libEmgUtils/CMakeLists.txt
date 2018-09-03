# Copyright: (C) 2017 Inria
# Author: Waldez Gomes
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(PROJECTNAME emgUtils)

project(${PROJECTNAME})

# include_directories(${YARP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} include)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} include)

add_library(${PROJECTNAME} SHARED emgutils.cpp)
target_include_directories(${PROJECTNAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES})

install(TARGETS ${PROJECTNAME} DESTINATION lib)

install(FILES emgutils.h DESTINATION include)
