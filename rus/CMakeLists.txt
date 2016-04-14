cmake_minimum_required(VERSION 2.6)

project(morphrus)
set(SOURCE libdict/capsheme.cpp libdict/flexmake.cpp libdict/lemmatiz.cpp libdict/mlmamain.cpp libdict/wildscan.cpp
           dictree/classmap.cpp dictree/flexTree.cpp dictree/lidstree.cpp dictree/mixTypes.cpp dictree/mxTables.cpp dictree/stemtree.cpp)
include_directories(libdict/ ../../)
add_library(morphrus STATIC ${SOURCE})

