cmake_minimum_required(VERSION 3.5)
project(morphrus)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(DicCPP  dictree/classmap.cpp
            dictree/stemtree.cpp
            dictree/lidstree.cpp
            dictree/flexTree.cpp
            dictree/mixTypes.cpp
            dictree/mxTables.cpp)

set(BuilDic morphrus-buildic)
set(MakeIch morphrus-makeich)

file(GLOB DicSrc ${morphrus_SOURCE_DIR}/dict/*.dic)

set (FlexTab tmp/flextabs.btf)
set (FlexIdx tmp/flextabs.sym)
set (FlexSrc ${morphrus_SOURCE_DIR}/tables/flextabs.tf)

set (IntrTab tmp/interchg.bmx)
set (IntrIdx tmp/interchg.sym)
set (IntrSrc ${morphrus_SOURCE_DIR}/tables/mtables.mx)

if (BUILD_SHARED_LIBS)
  set(SYMBOLS_FILE ${morphrus_SOURCE_DIR}/libdict/morphrus.unix.def)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE}")
endif(BUILD_SHARED_LIBS)

add_library(morphrus
  libdict/chartype.cpp
  libdict/mlmamain.cpp
  ${libmorph_SOURCE_DIR}/xmorph/capsheme.cpp
  ${libmorph_SOURCE_DIR}/xmorph/lemmatiz.cpp
  ${libmorph_SOURCE_DIR}/xmorph/wildscan.cpp
  ${DicCPP}
)

target_link_libraries(morphrus moonycode)

if (build-php)
  include(../cmake/FindPHP.cmake)

  if (NOT PHP_FOUND)
    message( SEND_ERROR "No PHP-devel found" )
    return()
  endif()

  include_directories( ${PHP_INCLUDE_DIRS} )

  set(CMAKE_CXX_FLAGS "-Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS "-DZEND_DEBUG=0")
  set(CMAKE_CXX_FLAGS "-DCOMPILE_DL_PHPMORPHRUS=1")

#  add_library(morphrus-php SHARED
  add_executable(morphrus-php
      php/phpapi.cpp

      libdict/chartype.cpp
      libdict/mlmamain.cpp
      ${libmorph_SOURCE_DIR}/xmorph/capsheme.cpp
      ${libmorph_SOURCE_DIR}/xmorph/lemmatiz.cpp
      ${libmorph_SOURCE_DIR}/xmorph/wildscan.cpp
      ${DicCPP}
  )

  target_link_libraries(morphrus-php
    moonycode
  )
endif()

add_custom_command(OUTPUT ${DicCPP}
	COMMAND ${BuilDic} -flex-table=${FlexTab} -flex-index=${FlexIdx}
		-intr-table=${IntrTab} -intr-index=${IntrIdx}
		-target-dir=dictree
		-unknown=dictree/unknown.txt
		-namespace=__libmorphrus__
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
	makeich/makeich.cpp)

add_executable(${BuilDic}
	buildic/lresolve.cpp
	buildic/makerus.cpp
	buildic/mtables.cpp)

target_link_libraries(${BuilDic}
	moonycode)

target_link_libraries(${MakeIch}
	moonycode)

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

add_executable(libmorph-rus-test
	libdict/test/testcapi.c
	libdict/test/testfunc.cpp)

target_link_libraries(libmorph-rus-test
     morphrus
     moonycode
     mtc)
