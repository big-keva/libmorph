find_path(MorphEng_INCLUDE
    NAMES eng.h
    PATHS /usr/local/include/libmorph /usr/include/libmorph)

find_library(MorphEng_LIB NAMES morpheng PATHS /usr/local/lib /usr/lib)
find_library(FuzzyEng_LIB NAMES fuzzyeng PATHS /usr/local/lib /usr/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MorphEng
    REQUIRED_VARS MorphEng_LIB FuzzyEng_LIB MorphEng_INCLUDE)

mark_as_advanced(MorphEng_INCLUDE MorphEng_LIB FuzzyEng_LIB)
