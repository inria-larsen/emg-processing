cmake_minimum_required(VERSION 3.0.2)
set(PROJECTNAME EMGhuman)
project(${PROJECTNAME})

#find_package (Qt5Widgets)

include_directories(${YARP_INCLUDE_DIRS} include)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

#add_library(mainwindow mainwindow.cpp)
#target_link_libraries (mainwindow Qt5::Widgets ${YARP_LIBRARIES})
    
#add_definitions(-D_USE_MATH_DEFINES)

add_executable(EMGhuman EMGhuman)
#target_link_libraries(EMGhuman ${YARP_LIBRARIES} mainwindow)
target_link_libraries(EMGhuman ${YARP_LIBRARIES})

install(TARGETS EMGhuman DESTINATION bin)
#install(TARGETS mainwindow DESTINATION lib)


