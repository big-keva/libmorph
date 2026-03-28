cmake_minimum_required(VERSION 3.5)

project(ENG_DICT
  DESCRIPTION "Dictionary-based morphological analyser for English"
  LANGUAGES CXX)

set(Namespace libmorph::eng)

add_compile_options(-DNAMESPACE=${Namespace})

set(DicCPP
  eng-dictree/classmap.cpp
  eng-dictree/stemtree.cpp
  eng-dictree/lidstree.cpp
  eng-dictree/flexTree.cpp)

set(BuilDic morpheng-buildic)

if(NOT DicSrc)
  file(GLOB DicSrc ${MORPHENG_SOURCE_DIR}/dict/*.dic)
endif()

if(MSVC)
  set(EXPORT_TABLE ${PROJECT_SOURCE_DIR}/morpheng.msvc.def)
else()
  set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/morpheng.unix.def)

  set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE} -static-libgcc -static-libstdc++" )
endif()

include_directories(.)

#
# create objects for building libraries
#
add_library(morpheng-objset OBJECT
  mlmamain.cpp
  buildform.cpp
  lemmatize.cpp
  ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
  ../chartype.cpp
  ${DicCPP})

set_target_properties(morphrus-objset PROPERTIES
  POSITION_INDEPENDENT_CODE ON)

#
# create shared library
#
add_library(morpheng-shared SHARED
  $<TARGET_OBJECTS:morpheng-objset>
  ${EXPORT_TABLE})

set_target_properties(morpheng-shared PROPERTIES 
  OUTPUT_NAME "morpheng")

target_link_libraries(morpheng-shared ${MoonyCode_LIB})

#
# create static library
#
add_library(morpheng-static STATIC
  $<TARGET_OBJECTS:morpheng-objset>)

set_target_properties(morpheng-static PROPERTIES
  OUTPUT_NAME "${STATIC_LIB_PREFIX}morpheng"
  CLEAN_DIRECT_OUTPUT 1)

#
# create the dictionary
#
add_custom_command(OUTPUT ${DicCPP}
  COMMAND ${BuilDic}
    -target-dir=eng-dictree
    -unknown=eng-dictree/unknown.txt
    -namespace=${Namespace}
    ${DicSrc}
    DEPENDS ${BuilDic} ${DicSrc} eng-dictree)

add_executable(${BuilDic}
  buildic/makeeng.cpp
  ../chartype.cpp)

# directories in build tree for custom built files:
#   * inflextion tables;
#   * dictionary sources.
add_custom_target(eng-dictree ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory eng-dictree)

add_executable(morpheng-test
  tests/test-eng.cpp)

target_link_libraries(morpheng-test ${MoonyCode_LIB} morpheng-objset mtc)
target_link_libraries(${BuilDic}    ${MoonyCode_LIB})

#
# install rus
#
install(
  TARGETS morpheng-shared
  LIBRARY DESTINATION lib COMPONENT EngRtm)

install(
  TARGETS morpheng-static
  ARCHIVE DESTINATION lib COMPONENT EngDev)
