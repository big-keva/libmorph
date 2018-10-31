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

set (LibType STATIC)

if (ENABLE_SHARED)
  set (LibType SHARED)
endif ()

add_library(morphrus ${LibType}
    libdict/chartype.cpp
    libdict/mlmamain.cpp
    ${libmorph_SOURCE_DIR}/xmorph/capsheme.cpp
    ${libmorph_SOURCE_DIR}/xmorph/lemmatiz.cpp
    ${libmorph_SOURCE_DIR}/xmorph/wildscan.cpp
    ${DicCPP}
)

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

if (ENABLE_SHARED)
  set(SYMBOLS_FILE ${morphrus_SOURCE_DIR}/libdict/morphrus.unix.def)

  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE}")

  target_link_libraries(morphrus
	moonycode)
endif()

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
