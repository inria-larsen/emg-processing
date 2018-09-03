cmake_minimum_required(VERSION 3.0.2)
set(PROJECTNAME EMGhuman)
project(${PROJECTNAME})

#include_directories(${YARP_INCLUDE_DIRS} include)

add_executable(EMGhuman EMGhuman)
target_link_libraries(EMGhuman ${YARP_LIBRARIES} emgUtils)

install(TARGETS EMGhuman DESTINATION bin)


