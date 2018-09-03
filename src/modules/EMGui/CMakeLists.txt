cmake_minimum_required(VERSION 2.8.12)

project(EMGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

#include_directories(${YARP_INCLUDE_DIRS} include)

add_executable(${PROJECT_NAME} "main.cpp" "emgui.cpp" "qml.qrc")

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Quick ${YARP_LIBRARIES} emgUtils)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
