cmake_minimum_required(VERSION 3.5)

project(ENG_PROB
  DESCRIPTION "Fuzzy morphological analyser for English"
  LANGUAGES CXX)

set(Namespace	libfuzzy::eng)

add_compile_options(-DNAMESPACE=${Namespace})

set(InflexCPP	fuzzydic.cpp)
set(InflexSRC	${PROJECT_SOURCE_DIR}/class/mclass.json)

set(MakeClasses	fuzzyeng-makeClasses)
set(CompileDict	fuzzyeng-compileDict)
set(MakePsTable	fuzzyeng-makePsTable)
	
if(MSVC)
  set(EXPORT_TABLE ${PROJECT_SOURCE_DIR}/fuzzyeng.msvc.def)
else()
  set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/fuzzyeng.unix.def)

  set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE} -static-libgcc -static-libstdc++" )
endif()

#
# create objects for building libraries
#
add_library(fuzzyeng-objset OBJECT
    mlfamain.cpp
    class/pspfid.cpp
    ${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
    ../chartype.cpp
    ${InflexCPP} )

set_target_properties(fuzzyeng-objset PROPERTIES
  POSITION_INDEPENDENT_CODE ON)

#
# create shared library
#
add_library(fuzzyeng-shared SHARED
  $<TARGET_OBJECTS:fuzzyeng-objset>
  ${EXPORT_TABLE})

set_target_properties(fuzzyeng-shared PROPERTIES 
  OUTPUT_NAME "fuzzyeng")

target_link_libraries(fuzzyeng-shared ${MoonyCode_LIB})

#
# create static library
#
add_library(fuzzyeng-static STATIC
  $<TARGET_OBJECTS:fuzzyeng-objset>)

set_target_properties(fuzzyeng-static PROPERTIES
  OUTPUT_NAME "${STATIC_LIB_PREFIX}fuzzyeng"
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

  add_custom_target(generate_mclass ALL
    DEPENDS "${InflexSRC}"
    COMMENT "Ensuring mclass.json exists")

  add_dependencies(fuzzyeng-objset generate_mclass)
endif()

#
# Компилятор словаря окончаний mclass.json
#
add_executable(${CompileDict}         tools/compile-dict.cpp)
target_link_libraries(${CompileDict}  ${MoonyCode_LIB} mtc)

#
# Генератор mclass.json - словарь грамматических классов
#
add_executable(${MakeClasses}         tools/make-classes.cpp)
target_link_libraries(${MakeClasses}  ${MoonyCode_LIB} morpheng-objset mtc)

#
# Статистика частей речи
#
if(DEVELOPER_MODE)
  add_executable(${MakePsTable}         tools/make-psp-tab.cpp)
  target_link_libraries(${MakePsTable}  ${MoonyCode_LIB} morpheng-objset mtc)
endif()

#
# Тесты
#
add_executable(fuzzyeng-test          tests/fuzzyeng-test.cpp)
target_link_libraries(fuzzyeng-test   ${MoonyCode_LIB} fuzzyeng-objset mtc)

#
# install eng
#
install(
  TARGETS fuzzyeng-shared
  LIBRARY DESTINATION lib COMPONENT EngRtm)

install(
  TARGETS fuzzyeng-static
  ARCHIVE DESTINATION lib COMPONENT EngDev)
