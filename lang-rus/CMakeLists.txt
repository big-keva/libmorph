cmake_minimum_required(VERSION 3.5)
project(MORPHRUS)

include_directories(./)

if(BUILD_MORPHRUS)
  add_subdirectory(libdict)
endif()

if(BUILD_FUZZYRUS)
  add_subdirectory(libprob)
endif()
