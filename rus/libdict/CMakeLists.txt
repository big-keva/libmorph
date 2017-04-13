cmake_minimum_required(VERSION 2.8)

project(morphrus)

add_definitions(-Wall -std=c++1y -pedantic)

include_directories(./ ../../ ../../../)

set(SOURCE_LIB chartype.cpp  mlmamain.cpp
  ../../xmorph/capsheme.cpp  ../../xmorph/lemmatiz.cpp  ../../xmorph/wildscan.cpp
  ../dictree/classmap.cpp  ../dictree/flexTree.cpp  ../dictree/lidstree.cpp
  ../dictree/mixTypes.cpp  ../dictree/mxTables.cpp  ../dictree/stemtree.cpp)

add_library(morphrus STATIC ${SOURCE_LIB})
