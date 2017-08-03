# Copyright: (C) 2017 Inria
# Author: Waldez Gomes
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(emgsignal)

find_library(EMG_TCP EmgTcp REQUIRED)

#${CMAKE_CURRENT_SOURCE_DIR}/../libEmgTcp/include

include_directories(${YARP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include/EmgSignal ${CMAKE_CURRENT_SOURCE_DIR}/../libEmgTcp/include include)


add_library(EmgSignal SHARED src/EmgSignal.cpp)


install(TARGETS EmgSignal DESTINATION lib)

install(DIRECTORY "include/" DESTINATION include)


