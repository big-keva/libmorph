cmake_minimum_required(VERSION 3.5)
project(LIBMORPH)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so") 

find_package(MoonyCode QUIET)

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
  contrib
  ${MoonyCode_INCLUDE_DIR})

#
# check if moonycode is available in build
#
if("${MoonyCode_LIB}" STREQUAL "" AND NOT TARGET moonycode-objset)
  add_subdirectory(contrib/moonycode)
  set(MoonyCode_LIB moonycode-objset)
endif()

#
# check if mtc is available in build
#
if (NOT TARGET mtc)
  add_subdirectory(contrib/mtc)
endif()

#
# languages
#
add_subdirectory(lang-rus)
add_subdirectory(lang-eng)

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
add_executable(libmorph-tfc
  utils/tfc/ftable.cpp
  utils/tfc/gramap.cpp
  utils/tfc/tfc.cpp)
target_link_libraries(libmorph-tfc ${MoonyCode_LIB})

#
# install API
#
install(
  FILES api.h api.hpp 
  DESTINATION include/libmorph 
  COMPONENT ApiDev)

#
# get package versions
#
file(READ "API_VER" VER_API)
  string(STRIP "${VER_API}" VER_API)

file(READ "lang-rus/dict/VERSION" VER_RUS)
  string(STRIP "${VER_RUS}" VER_RUS)
  set(VER_RUS ${VER_API}.${VER_RUS})

#
# CPack
#
set(CPACK_PACKAGE_NAME          "libmorph")
set(CPACK_PACKAGE_CONTACT       "Andrew Kovalenko <keva@rambler.ru>")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

add_subdirectory(pyrusmorph)
add_subdirectory(pyengmorph)

# Генераторы в зависимости от системы
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "DEB;RPM")
endif()

include(CPack)
