cmake_minimum_required(VERSION 3.5)
project(MORPHENG)

include_directories(./)

file(GLOB DicSrc ${MORPHENG_SOURCE_DIR}/dict/*.dic)

add_subdirectory(libdict)
#add_subdirectory(libprob)

install(
  FILES ../eng.h
  DESTINATION include/libmorph COMPONENT EngDev)

install(
  FILES cmake/MorphEngConfig.cmake
  DESTINATION lib/cmake/MorphEng COMPONENT EngDev)
