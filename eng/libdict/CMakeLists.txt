cmake_minimum_required(VERSION 3.5)

project(ENG_DICT
  VERSION 3.0.1
  DESCRIPTION "Dictionary-based morphological analyser for English"
  LANGUAGES CXX)

if (DEFINED MORPHENG_BUILD_SHARED_LIBS)
  if (MORPHENG_BUILD_SHARED_LIBS)
    set(MORPHENG_BUILD_AS_SHARED_LIBRARY "build libmorpheng as .so" ON)
  endif()
elseif(BUILD_SHARED_LIBS)
  if (NOT DEFINED MORPHENG_BUILD_SHARED_LIBS OR MORPHENG_BUILD_SHARED_LIBS)
    set(MORPHENG_BUILD_AS_SHARED_LIBRARY "build libmorpheng as .so" ON)
  endif()
endif()

set(Namespace libmorpheng)

set(DicCPP
  eng-dictree/classmap.cpp
  eng-dictree/stemtree.cpp
  eng-dictree/lidstree.cpp
  eng-dictree/flexTree.cpp)

add_compile_options(-W -Wall -Wextra)

set(BuilDic morpheng-buildic)

file(GLOB DicSrc ${MORPHENG_SOURCE_DIR}/dict/*.dic)

if (MORPHENG_BUILD_AS_SHARED_LIBRARY)

  set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/morpheng.unix.def)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE}")

  add_library(morpheng SHARED
    mlmamain.cpp
    buildform.cpp
    lemmatize.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
#    ${LIBMORPH_SOURCE_DIR}/xmorph/buildforms.cpp
    ${MORPHENG_SOURCE_DIR}/chartype.cpp
    ${DicCPP})

  target_link_libraries(morpheng moonycode)

else()

  add_library(morpheng STATIC
    mlmamain.cpp
    buildform.cpp
    lemmatize.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
#    ${LIBMORPH_SOURCE_DIR}/xmorph/buildforms.cpp
    ${MORPHENG_SOURCE_DIR}/chartype.cpp
    ${DicCPP})

endif()

add_custom_command(OUTPUT ${DicCPP}
	COMMAND ${BuilDic}
		-target-dir=eng-dictree
		-unknown=eng-dictree/unknown.txt
		-namespace=${Namespace}
		${DicSrc}
        DEPENDS ${BuilDic} ${DicSrc} eng-dictree
)

add_executable(${BuilDic}
	buildic/makeeng.cpp
	../chartype.cpp)

# directories in build tree for custom built files:
#   * inflextion tables;
#   * dictionary sources.
add_custom_target(eng-dictree ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory eng-dictree)

# install
install(TARGETS morpheng DESTINATION lib)

install(
  FILES
    ../include/mlma1033.h
  DESTINATION include/libmorph/eng/include)

add_executable(morpheng-test
	tests/test-eng.cpp)


target_link_libraries(morpheng-test
	morpheng
	moonycode
	mtc)

target_link_libraries(${BuilDic} moonycode)
