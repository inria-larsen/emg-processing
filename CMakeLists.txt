# Copyright: (C) 2017 Inria
# Author: Serena Ivaldi
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.0)
set(PROJECTNAME emg-processing)
project(${PROJECTNAME})
set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED YES)
set(CXX_EXTENSIONS NO)

find_package(YARP REQUIRED)
find_package(ICUB REQUIRED)
find_package(ICUBcontrib REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

add_subdirectory(src)
add_subdirectory(app)

icubcontrib_add_uninstall_target()
