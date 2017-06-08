cmake_minimum_required(VERSION 3.2)
project(morphrus)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
if(MSVC)
  add_compile_options(-MP)
else()
  add_compile_options(-W -Wall -Wextra)
endif()

include(ExternalProject)
set(LIBCODES_INSTALL_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/tiny_codepages)
ExternalProject_Add(tiny_codepages
  GIT_REPOSITORY https://github.com/big-keva/libcodes
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBCODES_INSTALL_LOCATION}
)

include_directories(
  ${LIBCODES_INSTALL_LOCATION}/include
  .
  rus/libdict
)

add_library(morphrus STATIC
  rus/dictree/classmap.cpp
  rus/dictree/flexTree.cpp
  rus/dictree/lidstree.cpp
  rus/dictree/mixTypes.cpp
  rus/dictree/mxTables.cpp
  rus/dictree/stemtree.cpp
  rus/libdict/chartype.cpp
  rus/libdict/mlmamain.cpp
  xmorph/capsheme.cpp
  xmorph/lemmatiz.cpp
  xmorph/wildscan.cpp
)

add_dependencies(morphrus tiny_codepages)
install(TARGETS morphrus DESTINATION lib)

add_subdirectory(rus/libdict/test)
