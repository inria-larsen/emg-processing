cmake_minimum_required(VERSION 2.8)
set(PROJECTNAME EMGhuman)
project(${PROJECTNAME})

include_directories(${YARP_INCLUDE_DIRS} include)
    
#add_definitions(-D_USE_MATH_DEFINES)

add_executable(EMGhuman EMGhuman)
target_link_libraries(EMGhuman ${YARP_LIBRARIES})

install(TARGETS EMGhuman DESTINATION bin)


