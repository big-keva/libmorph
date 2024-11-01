cmake_minimum_required(VERSION 3.5)

project(RUS_DICT
  VERSION 3.0.1
  DESCRIPTION "Dictionary-based morphological analyser for Russian"
  LANGUAGES CXX)

if (DEFINED MORPHRUS_BUILD_SHARED_LIBS)
  if (MORPHRUS_BUILD_SHARED_LIBS)
    set(MORPHRUS_BUILD_AS_SHARED_LIBRARY "build libmorphrus as .so" ON)
  endif()
elseif(BUILD_SHARED_LIBS)
  if (NOT DEFINED MORPHRUS_BUILD_SHARED_LIBS OR MORPHRUS_BUILD_SHARED_LIBS)
    set(MORPHRUS_BUILD_AS_SHARED_LIBRARY "build libmorphrus as .so" ON)
  endif()
endif()

set(Namespace libmorphrus)

set(DicCPP
  dictree/classmap.cpp
  dictree/stemtree.cpp
  dictree/lidstree.cpp
  dictree/flexTree.cpp
  dictree/mixTypes.cpp
  dictree/mxTables.cpp)

add_compile_options(-W -Wall -Wextra)

set(BuilDic morphrus-buildic)
set(MakeIch morphrus-makeich)

file(GLOB DicSrc ${MORPHRUS_SOURCE_DIR}/dict/*.dic)

set (FlexTab tmp/flextabs.btf)
set (FlexIdx tmp/flextabs.sym)
set (FlexSrc ${MORPHRUS_SOURCE_DIR}/tables/flextabs.tf)

set (IntrTab tmp/interchg.bmx)
set (IntrIdx tmp/interchg.sym)
set (IntrSrc ${MORPHRUS_SOURCE_DIR}/tables/mtables.mx)

if (MORPHRUS_BUILD_AS_SHARED_LIBRARY)
  set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/morphrus.unix.def)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE}")

  add_library(morphrus SHARED
    mlmamain.cpp
    buildform.cpp
    lemmatize.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/buildforms.cpp
    ${MORPHRUS_SOURCE_DIR}/chartype.cpp
    ${DicCPP})

  target_link_libraries(morphrus moonycode)

else()

  add_library(morphrus STATIC
    mlmamain.cpp
    buildform.cpp
    lemmatize.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/buildforms.cpp
    ${MORPHRUS_SOURCE_DIR}/chartype.cpp
    ${DicCPP})

endif()

add_custom_command(OUTPUT ${DicCPP}
	COMMAND ${BuilDic} -flex-table=${FlexTab} -flex-index=${FlexIdx}
		-intr-table=${IntrTab} -intr-index=${IntrIdx}
		-target-dir=dictree
		-unknown=dictree/unknown.txt
		-namespace=${Namespace}
		${DicSrc}
        DEPENDS ${BuilDic} ${DicSrc} ${FlexTab} ${FlexIdx} ${IntrTab} ${IntrIdx} dictree
)

add_custom_command(OUTPUT ${FlexTab} ${FlexIdx}
	COMMAND tfc -lang:rus ${FlexSrc} ${FlexTab} ${FlexIdx}
	DEPENDS tfc ${FlexSrc} tmp)

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

# install
install(TARGETS morphrus DESTINATION lib)

install(
  FILES
    include/mlma1049.h
  DESTINATION include/libmorph/rus/include)

add_executable(morphrus-test
	tests/testcapi.c
	tests/testfunc.cpp)


target_link_libraries(morphrus-test
	morphrus
	moonycode
	mtc)

target_link_libraries(${BuilDic}	moonycode)
target_link_libraries(${MakeIch}        moonycode)
