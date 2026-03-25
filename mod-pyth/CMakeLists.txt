cmake_minimum_required(VERSION 3.12)
project(mod-pyth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# need package pybind11;
# pip install pybind11
#   ||
# sudo apt install python3-pybind11
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

# python module name is 'pyrusmorph'
pybind11_add_module(pyrusmorph	rus.cpp)

target_link_libraries(pyrusmorph PRIVATE
	morphrus-static
	fuzzyrus-static
	moonycode)

# remove 'lib' prefix
set_target_properties(pyrusmorph PROPERTIES PREFIX "")
