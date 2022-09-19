cmake_minimum_required(VERSION 3.5)
project(libmorph)

option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(build-php "Build shared library" OFF)

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

add_subdirectory(rus)

add_executable(mapidl mapidl/mapidl.cpp)

add_executable(tfc
  tfc/ftable.cpp
  tfc/gramap.cpp
  tfc/tfc.cpp)

target_link_libraries(tfc moonycode)
