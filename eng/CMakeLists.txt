cmake_minimum_required(VERSION 3.5)

project(MORPHENG)

include_directories(./)

if(BUILD_MORPHENG)
  add_subdirectory(libdict)
endif()

if(BUILD_FUZZYENG)
# add_subdirectory(libprob)
endif()
