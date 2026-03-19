cmake_minimum_required(VERSION 3.5)

project(RUS_PROB
  DESCRIPTION "Fuzzy morphological analyser for Russian"
  LANGUAGES CXX)

set(Namespace	libfuzzyrus)

set(InflexCPP	fuzzydic.cpp)
set(InflexSRC	${PROJECT_SOURCE_DIR}/class/mclass.json)

set(MakeClasses	fuzzyrus-makeClasses)
set(CompileDict	fuzzyrus-compileDict)
set(MakePsTable	fuzzyrus-makePsTable)
	
if(MSVC)
  set(EXPORT_TABLE ${PROJECT_SOURCE_DIR}/fuzzyrus.msvc.def)
else()
  set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/fuzzyrus.unix.def)

  set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE} -static-libgcc -static-libstdc++" )
endif()

#
# create objects for building libraries
#
add_library(fuzzyrus-objset OBJECT
    mlfamain.cpp
    class/pspfid.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
    ../chartype.cpp
    ${InflexCPP} )

set_target_properties(fuzzyrus-objset PROPERTIES
  POSITION_INDEPENDENT_CODE ON)

#
# create shared library
#
add_library(fuzzyrus-shared SHARED
  $<TARGET_OBJECTS:fuzzyrus-objset>
  ${EXPORT_TABLE})

set_target_properties(fuzzyrus-shared PROPERTIES 
  OUTPUT_NAME "fuzzyrus")

target_link_libraries(fuzzyrus-shared ${MoonyCode_LIBRARY})

#
# create static library
#
add_library(fuzzyrus-static STATIC
  $<TARGET_OBJECTS:fuzzyrus-objset>)

set_target_properties(fuzzyrus-static PROPERTIES
  OUTPUT_NAME "${STATIC_LIB_PREFIX}fuzzyrus"
  CLEAN_DIRECT_OUTPUT 1)

#
# Динамически генерируемый fuzzydic.cpp делается из class/mclass.json;
# Есть способ его динамической генерации (если его нет), но по умолчанию
# используем существующий, так как может быть поправлен вручную.
#
add_custom_command(OUTPUT ${InflexCPP}
  COMMAND ${CompileDict} -nspace=${Namespace} ${InflexSRC} ${InflexCPP}
  DEPENDS ${CompileDict} ${InflexSRC} )

#
# На случай, если нет class/mclass.json, можно сгенерировать его автоматически
# на основе словарного морфоанализатора и заданного предела покрытия лексики
#

if(NOT EXISTS "${InflexSRC}")
  add_custom_command(
    OUTPUT "${InflexSRC}"
    COMMAND ${MakeClasses} ${DicSrc} > "${InflexSRC}"
    COMMENT "Generating ${InflexSRC} using {${DicSrc}} because it is missing")
endif()

#
# Компилятор словаря окончаний mclass.json
#
add_executable(${CompileDict}         tools/compile-dict.cpp)
target_link_libraries(${CompileDict}  ${MoonyCode_LIBRARY} mtc)

#
# Генератор mclass.json - словарь грамматических классов
#
add_executable(${MakeClasses}         tools/make-classes.cpp)
target_link_libraries(${MakeClasses}  ${MoonyCode_LIBRARY} morphrus-objset mtc)

#
# Статистика частей речи
#
if(DEVELOPER_MODE)
  add_executable(${MakePsTable}         tools/make-psp-tab.cpp)
  target_link_libraries(${MakePsTable}  ${MoonyCode_LIBRARY} morphrus-objset mtc)
endif()

#
# Тесты
#
add_executable(fuzzyrus-test          tests/fuzzyrus-test.cpp)
target_link_libraries(fuzzyrus-test   ${MoonyCode_LIBRARY} fuzzyrus-objset mtc)

#
# install rus
#
install(
  TARGETS fuzzyrus-shared
  LIBRARY DESTINATION lib COMPONENT RusRtm)

install(
  TARGETS fuzzyrus-static
  ARCHIVE DESTINATION lib COMPONENT RusDev)
