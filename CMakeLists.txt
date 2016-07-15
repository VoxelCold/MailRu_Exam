cmake_minimum_required(VERSION 2.8)

project(final)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (LIBS "-pthread")

add_executable(${PROJECT_NAME} server.cpp socket.cpp)

target_link_libraries(${PROJECT_NAME} ${LIBS})
