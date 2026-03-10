cmake_minimum_required(VERSION 3.5)

project(RUS_DICT
  DESCRIPTION "Dictionary-based morphological analyser for Russian"
  LANGUAGES CXX)

message(STATUS "libmorph:rus building enabled, version ${API_VERSION}.${MORPHRUS_DICT_VER}.${MORPHRUS_CODE_VER}")

set(Namespace libmorphrus)

set(DicCPP
  dictree/classmap.cpp
  dictree/stemtree.cpp
  dictree/lidstree.cpp
  dictree/flexTree.cpp
  dictree/mixTypes.cpp
  dictree/mxTables.cpp)

add_compile_options(-fPIC)

set(BuilDic morphrus-buildic)
set(MakeIch morphrus-makeich)

file(GLOB DicSrc ${MORPHRUS_SOURCE_DIR}/dict/*.dic)

set (FlexTab tmp/flextabs.btf)
set (FlexIdx tmp/flextabs.sym)
set (FlexSrc ${MORPHRUS_SOURCE_DIR}/tables/flextabs.tf)

set (IntrTab tmp/interchg.bmx)
set (IntrIdx tmp/interchg.sym)
set (IntrSrc ${MORPHRUS_SOURCE_DIR}/tables/mtables.mx)

set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/morphrus.unix.def)

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE} -static-libgcc -static-libstdc++" )

include_directories(.)

#
# create objects for building libraries
#
add_library(morphrus-objset OBJECT
    mlmamain.cpp
    buildform.cpp
    lemmatize.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/buildforms.cpp
    ../chartype.cpp
    ${DicCPP})

set_target_properties(morphrus-objset PROPERTIES
    POSITION_INDEPENDENT_CODE ON)

if(DEV)
  #
  # create static library
  #
  add_library(morphrus-static STATIC
    $<TARGET_OBJECTS:morphrus-objset>)

  set_target_properties(morphrus-static PROPERTIES
    OUTPUT_NAME "morphrus"
    CLEAN_DIRECT_OUTPUT 1)
else()
  #
  # create shared library
  #
  add_library(morphrus-shared SHARED
    $<TARGET_OBJECTS:morphrus-objset>)

  set_target_properties(morphrus-shared PROPERTIES 
    OUTPUT_NAME "morphrus")

  target_link_libraries(morphrus-shared
    morphrus-objset
    moonycode)
endif()

#
# create the dictionary
#
add_custom_command(OUTPUT ${DicCPP}
	COMMAND ${BuilDic} -flex-table=${FlexTab} -flex-index=${FlexIdx}
		-intr-table=${IntrTab} -intr-index=${IntrIdx}
		-target-dir=dictree
		-unknown=dictree/unknown.txt
		-namespace=${Namespace}
		${DicSrc}
        DEPENDS ${BuilDic} ${DicSrc} ${FlexTab} ${FlexIdx} ${IntrTab} ${IntrIdx} dictree
)

#
# compile flexion tables
#
add_custom_command(OUTPUT ${FlexTab} ${FlexIdx}
	COMMAND libmorph-tfc -lang:rus ${FlexSrc} ${FlexTab} ${FlexIdx}
	DEPENDS libmorph-tfc ${FlexSrc} tmp)

#
# compile stem interchanges
#
add_custom_command(OUTPUT ${IntrTab} ${IntrIdx}
  COMMAND ${MakeIch} ${IntrSrc} ${IntrTab} ${IntrIdx}
  DEPENDS ${MakeIch} ${IntrSrc} tmp)

add_executable(${MakeIch}
  tools/makeich/makeich.cpp)

add_executable(${BuilDic}
  tools/buildic/lresolve.cpp
  tools/buildic/makerus.cpp
  tools/buildic/mtables.cpp)

# directories in build tree for custom built files:
#   * inflextion tables;
#   * dictionary sources.
add_custom_target(dictree ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory dictree)

add_custom_target(tmp ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory tmp)

add_executable(morphrus-test
  tests/testcapi.c
  tests/testfunc.cpp)

target_link_libraries(morphrus-test
  morphrus-objset
  moonycode
  mtc)

target_link_libraries(${BuilDic} moonycode)
target_link_libraries(${MakeIch} moonycode)


#
# install section
#
if(DEV)
  install(TARGETS morphrus-static
    ARCHIVE DESTINATION lib)
  install(
    FILES ../../rus.h
    DESTINATION include/libmorph)
else()
  install(TARGETS morphrus-shared
    LIBRARY DESTINATION lib)
endif()
