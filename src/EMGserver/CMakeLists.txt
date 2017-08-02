cmake_minimum_required(VERSION 2.8)
set(PROJECTNAME EMGserver)
project(${PROJECTNAME})

include_directories(${YARP_INCLUDE_DIRS} include)
    
#add_definitions(-D_USE_MATH_DEFINES)

add_executable(EMGserver EMGserver.cpp)
target_link_libraries(EMGserver ${YARP_LIBRARIES})

install(TARGETS EMGserver DESTINATION bin)


