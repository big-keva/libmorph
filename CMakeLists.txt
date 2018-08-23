cmake_minimum_required(VERSION 3.5)
project(libmorph)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include_directories(
  .
  contrib
  rus/libdict)

add_subdirectory(
  contrib/moonycode)

add_subdirectory(
  rus)

add_executable(mapidl
  mapidl/mapidl.cpp)

add_executable(tfc
  tfc/ftable.cpp
  tfc/gramap.cpp
  tfc/tfc.cpp)

target_link_libraries(tfc
  moonycode)

