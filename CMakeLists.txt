cmake_minimum_required(VERSION 3.5)
project(LIBMORPH)

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
if(NOT DEFINED BUILD_MORPHRUS)  
  set(BUILD_MORPHRUS ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
endif()

if(NOT DEFINED BUILD_FUZZYRUS)  
  set(BUILD_FUZZYRUS ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
endif()

if(NOT DEFINED BUILD_MORPHENG)  
  set(BUILD_MORPHENG ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
endif()

if(NOT DEFINED BUILD_FUZZYENG)  
  set(BUILD_FUZZYENG ${DEVELOPER_MODE}  CACHE BOOL "" FORCE)
endif()

#  set(BUILD_MORPHUKR ${DEVELOPER_MODE} CACHE BOOL "" FORCE)
#  set(BUILD_FUZZYUKR ${DEVELOPER_MODE} CACHE BOOL "" FORCE)

message(STATUS "DEVELOPER_MODE=${DEVELOPER_MODE}\n"
  "\tBUILD_MORPHRUS=${BUILD_MORPHRUS}\n"
  "\tBUILD_FUZZYRUS=${BUILD_FUZZYRUS}\n"
  "\tBUILD_MORPHENG=${BUILD_MORPHENG}\n"
  "\tBUILD_FUZZYENG=${BUILD_FUZZYENG}")

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
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-W -Wall -Wextra)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O0)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif()

include_directories(
  ./
  contrib)

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

#
# enable RUS components
#
if(BUILD_MORPHRUS OR BUILD_FUZZYRUS)
  message(STATUS "libmorph:rus building enabled")
  add_subdirectory(lang-rus)
endif()

#
# enable ENG components
#
if(BUILD_MORPHENG OR BUILD_FUZZYENG)
  message(STATUS "libmorph:eng building enabled")
  add_subdirectory(lang-eng)
endif()

#if(BUILD_MORPHUKR OR BUILD_FUZZYUKR)
#  message(STATUS "libmorph:rus building enabled")
#  add_subdirectory(ukr)
#endif()

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
# install section
#
install(
  FILES morphapi.h fuzzyapi.h
  DESTINATION include/libmorph
  COMPONENT API)

#
# CPack
#
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

set(CPACK_DEBIAN_API_PACKAGE_ARCHITECTURE "all")

#
# package manifest
#
set(CPACK_PACKAGE_NAME "libmorph")
set(CPACK_PACKAGE_VERSION "4.202602")
set(CPACK_PACKAGE_CONTACT "Andrey Kovalenko aka Keva <keva@rambler.ru>")

#
# dependencies
#
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6") 

include(CPack)
