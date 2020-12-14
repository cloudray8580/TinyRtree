cmake_minimum_required(VERSION 3.16)
project(MyRtree)

set(CMAKE_CXX_STANDARD 17)

add_executable(MyRtree main.cpp TinyRtree.h)

target_link_libraries(MyRtree ${CMAKE_DL_LIBS})