cmake_minimum_required(VERSION 2.8)
set(PROJECTNAME EMGhuman2robot)
project(${PROJECTNAME})

include_directories(${YARP_INCLUDE_DIRS} include)
    
#add_definitions(-D_USE_MATH_DEFINES)

add_executable(EMGhuman2robot EMGhuman2robot.cpp)
target_link_libraries(EMGhuman2robot ${YARP_LIBRARIES})

install(TARGETS EMGhuman2robot DESTINATION bin)


