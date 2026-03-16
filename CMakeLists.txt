cmake_minimum_required(VERSION 3.5)
project(LIBMORPH)

#
# check configuration
#
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
  set(STATIC_LIB_PREFIX "lib")
endif()

#
# DEVELOPER_MODE enforces building all the targets in CMakeLists;
#
# if !defined, try define it according to the building environment:
#   CI - no default targets;
#   CLION_IDE - all targets enabled.
#
if(NOT DEFINED DEVELOPER_MODE)
  message(STATUS "'DEVELOPER_MODE' it not set, try detect the build environment...")

  if(DEFINED ENV{CLION_IDE})
    message(STATUS "Clion build environment detected")

    set(DEVELOPER_MODE TRUE CACHE BOOL "" FORCE)
  else()
    set(DEVELOPER_MODE FALSE CACHE BOOL "" FORCE)
  endif()
endif()

#
# apply DEVELOPER_MODE to undefined targets
#
if(NOT DEFINED BUILD_MORPHAPI)  
  set(BUILD_MORPHAPI ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
endif()

if(NOT DEFINED BUILD_MORPHRUS)  
  set(BUILD_MORPHRUS ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
endif()

if(NOT DEFINED BUILD_MORPHENG)  
  set(BUILD_MORPHENG ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
endif()

#
# check for build any code
#
if(BUILD_MORPHRUS OR BUILD_MORPHENG OR DEVELOPER_MODE)
  set(BUILD_ANY_CODE ON)
else()
  set(BUILD_ANY_CODE OFF)
endif()

#
# set output directories
#
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

# Enable C & C++.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(NOT MSVC)
  add_compile_options(-W -Wall -Wextra)
  add_compile_options(-fPIC)

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0)
  else()
    add_compile_options(-fstack-protector-all -g)
  endif()
endif()

include_directories(
  ./
  contrib)

if(BUILD_ANY_CODE)
  #
  # check if moonycode is available in build
  #
  if (NOT TARGET moonycode)
    add_subdirectory(contrib/moonycode)
  endif()

  #
  # check if mtc is available in build
  #
  if (NOT TARGET mtc)
    add_subdirectory(contrib/mtc)
  endif()
endif()

#
# enable RUS components
#
if(BUILD_MORPHRUS)
  message(STATUS "libmorph:rus building enabled")

  add_subdirectory(lang-rus)
endif()

#
# enable ENG components
#
if(BUILD_MORPHENG)
  message(STATUS "libmorph:eng building enabled")

  add_subdirectory(lang-eng)
endif()

#
# build utilities only in developer mode
#
if(DEVELOPER_MODE)
  message(STATUS "libmorph:utils building enabled")

  add_subdirectory(utils)
endif()

#
# check if tfc is needed
#
if(BUILD_MORPHRUS OR BUILD_MORPHUKR)
  message(STATUS "libmorph:tfc building enabled")

  add_executable(libmorph-tfc
    utils/tfc/ftable.cpp
    utils/tfc/gramap.cpp
    utils/tfc/tfc.cpp)
  target_link_libraries(libmorph-tfc moonycode)
endif()

#
# package libmorph-api
#
if(BUILD_MORPHAPI)
  add_custom_target(morphapi ALL
    COMMAND ${CMAKE_COMMAND} -E echo "API headers are ready for installation")
    
  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/morphapi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzzyapi.h
    DESTINATION include/libmorph)
    
  message(STATUS "API headers will be installed to include/api")
else()
  message(STATUS "API headers are disabled")
endif()
