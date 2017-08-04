cmake_minimum_required(VERSION 3.0)
set(PROJECTNAME EMGserver)
project(${PROJECTNAME})

find_library(EMG_TCP EmgTcp REQUIRED)
find_library(EMG_SIGNAL EmgSignal REQUIRED)
set(LIB_EMG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/libEmgSignal/include
		${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/libEmgTcp/include )

include_directories(${YARP_INCLUDE_DIRS} ${LIB_EMG_DIR} include)
    
#add_definitions(-D_USE_MATH_DEFINES)

add_executable(EMGserver EMGserver.cpp)
target_link_libraries(EMGserver ${YARP_LIBRARIES} ${EMG_TCP} ${EMG_SIGNAL})

install(TARGETS EMGserver DESTINATION bin)


