cmake_minimum_required(VERSION 3.23)
project(Lights_windows)

set(CMAKE_CXX_STANDARD 14)


add_executable(Lights_windows main.cpp request.h)

target_link_libraries(Lights_windows winhttp)