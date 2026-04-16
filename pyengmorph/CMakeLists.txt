cmake_minimum_required(VERSION 3.12)
project(pyengmorph)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

include_directories(../)

# python module name is 'pyrusmorph'
pybind11_add_module(pyengmorph	eng.cpp)

target_link_libraries(pyengmorph PRIVATE
	morpheng-static
	fuzzyeng-static
	moonycode)

# remove 'lib' prefix
set_target_properties(pyengmorph PROPERTIES PREFIX "")
