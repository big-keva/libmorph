find_path(MorphRus_INCLUDE
    NAMES rus.h
    PATHS /usr/local/include/libmorph /usr/include/libmorph)

find_library(MorphRus_LIB NAMES morphrus PATHS /usr/local/lib /usr/lib)
find_library(FuzzyRus_LIB NAMES fuzzyrus PATHS /usr/local/lib /usr/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MorphRus
    REQUIRED_VARS MorphRus_LIB FuzzyRus_LIB MorphRus_INCLUDE)

mark_as_advanced(MorphRus_INCLUDE MorphRus_LIB FuzzyRus_LIB)
