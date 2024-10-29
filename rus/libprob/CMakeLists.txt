cmake_minimum_required(VERSION 3.5)

project(LIBFUZZYRUS)

set(Namespace	libfuzzyrus)

set(InflexCPP	fuzzydic.cpp)
set(InflexSRC	${PROJECT_SOURCE_DIR}/class/inflex.json)

add_compile_options(-W -Wall -Wextra)

set(MakeClasses	fuzzyrus-makeClasses)
set(CompileDict	fuzzyrus-compile-dic)
	
if (BUILD_SHARED_LIBS)
  set(SYMBOLS_FILE ${PROJECT_SOURCE_DIR}/fuzzyrus.unix.def)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-retain-symbols-file -Wl,${SYMBOLS_FILE}")
  set(LIBRARY_TYPE SHARED)
endif(BUILD_SHARED_LIBS)

add_library(fuzzyrus ${LIBRARY_TYPE}
	mlfamain.cpp
	${LIBMORPH_SOURCE_DIR}/xmorph/capsheme.cpp
  	${MORPHRUS_SOURCE_DIR}/chartype.cpp
	${InflexCPP} )
target_link_libraries(fuzzyrus
	moonycode)

add_executable(fuzzyrus-test
	tests/fuzzyrus-test.cpp)
target_link_libraries(fuzzyrus-test
	fuzzyrus moonycode mtc)

#
# Динамически генерируемый fuzzydic.cpp делается из class/inflex.json;
# Есть способ его динамической генерации (если его нет), но по умолчанию
# используем существующий, так как может быть поправлен вручную.
#
add_custom_command(OUTPUT ${InflexCPP}
	COMMAND ${CompileDict} -nspace=${Namespace} ${InflexSRC} ${InflexCPP}
        DEPENDS ${CompileDict} ${InflexSRC} )

#
# На случай, если нет class/inflex.json, можно сгенерировать его автоматически
# на основе словарного морфоанализатора и заданного предела покрытия лексики
#
add_custom_command(OUTPUT ${InflexSRC}
	COMMAND ${MakeClasses} > ${InflexSRC}
        DEPENDS ${MakeClasses} )

#
# Компилятор словаря окончаний inflex.json
#
add_executable(${CompileDict}	tools/compile-dict.cpp)

#
# Генератор inflex.json - словарь грамматических классов
#
add_executable(${MakeClasses}	tools/make-classes.cpp)

target_link_libraries(${CompileDict}	moonycode mtc)
target_link_libraries(${MakeClasses}	morphrus moonycode mtc)
