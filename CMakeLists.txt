cmake_minimum_required(VERSION 3.5)
project(libmorph)

OPTION(build-static "build static library" OFF)
OPTION(build-shared "build shared library" OFF)
OPTION(build-php "build shared library" OFF)

if (NOT build-static AND NOT build-shared AND NOT build-php)
    message(WARNING "No targets of 'build-static', 'build-shared' or 'build-php' specified, using default 'build-static'.")
    SET(build-static ON)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_compile_options(-W -Wall -Wextra)
add_compile_options(-O3)

include_directories(
  .
  contrib
  rus/libdict)

if (NOT TARGET moonycode)
  add_subdirectory(contrib/moonycode)
endif()

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
