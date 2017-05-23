cmake_minimum_required(VERSION 3.5)
project(morphrus)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
if(MSVC)
  add_compile_options(-MP)
else()
  add_compile_options(-W -Wall -Wextra)
endif()

include_directories(
  .
  rus/libdict
)

add_library(big_keva_morphrus STATIC
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

install(TARGETS big_keva_morphrus DESTINATION lib)
