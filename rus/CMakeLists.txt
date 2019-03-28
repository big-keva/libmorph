cmake_minimum_required(VERSION 3.5)
project(morphrus)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set (DicCPP dictree/classmap.cpp
            dictree/stemtree.cpp
            dictree/lidstree.cpp
            dictree/flexTree.cpp
            dictree/mixTypes.cpp
            dictree/mxTables.cpp)

file(GLOB DicSrc ${morphrus_SOURCE_DIR}/dict/*.dic)

set (FlexTab tmp/flextabs.btf)
set (FlexIdx tmp/flextabs.sym)
set (FlexSrc ${morphrus_SOURCE_DIR}/tables/flextabs.tf)

set (IntrTab tmp/interchg.bmx)
set (IntrIdx tmp/interchg.sym)
set (IntrSrc ${morphrus_SOURCE_DIR}/tables/mtables.mx)

if (build-static)
  add_library(morphrus STATIC
      libdict/chartype.cpp
      libdict/mlmamain.cpp
      ${libmorph_SOURCE_DIR}/xmorph/capsheme.cpp
      ${libmorph_SOURCE_DIR}/xmorph/lemmatiz.cpp
      ${libmorph_SOURCE_DIR}/xmorph/wildscan.cpp
      ${DicCPP}
  )
endif ()

if (build-shared)
  set(SYMBOLS_FILE ${morphrus_SOURCE_DIR}/libdict/morphrus.unix.def)

  add_library(morphrus-sh SHARED
      libdict/chartype.cpp
      libdict/mlmamain.cpp
      ${libmorph_SOURCE_DIR}/xmorph/capsheme.cpp
      ${libmorph_SOURCE_DIR}/xmorph/lemmatiz.cpp
      ${libmorph_SOURCE_DIR}/xmorph/wildscan.cpp
      ${DicCPP}
  )

  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE}")

  target_link_libraries(morphrus-sh
	moonycode)
endif()

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
	COMMAND makerus -flex-table=${FlexTab} -flex-index=${FlexIdx}
			-intr-table=${IntrTab} -intr-index=${IntrIdx}
			-target-dir=dictree
			-unknown=dictree/unknown.txt
			-namespace=__libmorphrus__
			${DicSrc}
        DEPENDS makerus ${DicSrc} ${FlexTab} ${FlexIdx} ${IntrTab} ${IntrIdx} dictree
)

add_custom_command(OUTPUT ${FlexTab} ${FlexIdx}
	COMMAND tfc -lang:rus ${FlexSrc} ${FlexTab} ${FlexIdx}
	DEPENDS tfc ${FlexSrc} tmp)

add_custom_command(OUTPUT ${IntrTab} ${IntrIdx}
	COMMAND makeich ${IntrSrc} ${IntrTab} ${IntrIdx}
	DEPENDS makeich ${IntrSrc} tmp)

add_executable(makeich
	makeich/makeich.cpp)

add_executable(makerus
	buildic/lresolve.cpp
	buildic/makerus.cpp
	buildic/mtables.cpp)

target_link_libraries(makerus
	moonycode)

target_link_libraries(makeich
	moonycode)

# directories in build tree for custom built files:
#   * inflextion tables;
#   * dictionary sources.
add_custom_target(dictree ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory dictree)

add_custom_target(tmp ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory tmp)

# install
if (build-static)
	install(TARGETS morphrus DESTINATION lib)
endif()

if (build-shared)
	install(TARGETS morphrus-sh DESTINATION lib)
endif()

install(
  FILES
    include/mlma1049.h
  DESTINATION include/libmorph/rus/include)
