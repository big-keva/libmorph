cmake_minimum_required(VERSION 3.5)
project(MORPHRUS)

include_directories(./)

file(GLOB DicSrc ${MORPHRUS_SOURCE_DIR}/dict/*.dic)

add_subdirectory(libdict)
add_subdirectory(libprob)

install(
  FILES ../rus.h
  DESTINATION include/libmorph COMPONENT RusDev)

install(
  FILES cmake/MorphRusConfig.cmake
  DESTINATION lib/cmake/MorphRus COMPONENT RusDev)
