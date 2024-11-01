cmake_minimum_required(VERSION 3.5)
project(LIBMORPH)

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all")
endif()

if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single Directory for all")
endif()

if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single Directory for all")
endif()

# Enable C & C++.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-W -Wall -Wextra)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O0)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif()

include_directories(
  ./
  contrib
  rus/libdict)

if (NOT TARGET moonycode)
  add_subdirectory(contrib/moonycode)
endif()

if (NOT TARGET mtc)
  add_subdirectory(contrib/mtc)
endif()

add_subdirectory(rus)
# add_subdirectory(ukr)

add_executable(mapidl mapidl/mapidl.cpp)

add_executable(tfc
  tfc/ftable.cpp
  tfc/gramap.cpp
  tfc/tfc.cpp)
target_link_libraries(tfc moonycode)
