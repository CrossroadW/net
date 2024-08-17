cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
project(example)
add_executable(${CMAKE_PROJECT_NAME}_server main.cpp)
add_executable(${CMAKE_PROJECT_NAME}_client client.cpp)

