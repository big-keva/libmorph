cmake_minimum_required(VERSION 3.12)
project(python_pad_rus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# need package pybind11;
# pip install pybind11
#   ||
# sudo apt install python3-pybind11
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

# python module name is 'libmorphrus-py'
pybind11_add_module(libmorphrus	morphrus-pad.cpp)

pybind11_add_module(libfuzzyrus	fuzzyrus-pad.cpp)

target_link_libraries(libmorphrus PRIVATE
	morphrus
	moonycode)
target_link_libraries(libfuzzyrus PRIVATE
	fuzzyrus
	moonycode)

# remove 'lib' prefix
set_target_properties(libmorphrus PROPERTIES PREFIX "")
set_target_properties(libfuzzyrus PROPERTIES PREFIX "")
