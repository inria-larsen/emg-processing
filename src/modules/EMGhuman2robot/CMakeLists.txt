cmake_minimum_required(VERSION 3.5)
set(PROJECTNAME EMGhuman2robot)
project(${PROJECTNAME})

#include_directories(${YARP_INCLUDE_DIRS} include)

add_executable(EMGhuman2robot EMGhuman2robot.cpp robot_interfaces.cpp)
target_link_libraries(EMGhuman2robot ${YARP_LIBRARIES} emgUtils)

install(TARGETS EMGhuman2robot DESTINATION bin)


