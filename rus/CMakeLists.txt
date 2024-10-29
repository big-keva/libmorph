cmake_minimum_required(VERSION 3.5)

project(MORPHRUS)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

include_directories(./)

add_subdirectory(libdict)
add_subdirectory(libprob)
