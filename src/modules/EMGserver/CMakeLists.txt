cmake_minimum_required(VERSION 3.0)
set(PROJECTNAME EMGserver)
project(${PROJECTNAME})

#include_directories(${YARP_INCLUDE_DIRS} include)

add_executable(EMGserver EMGserver.cpp EmgTcp.cpp EmgSignal.cpp)

target_link_libraries(EMGserver ${YARP_LIBRARIES} emgUtils )

install(TARGETS EMGserver DESTINATION bin)


