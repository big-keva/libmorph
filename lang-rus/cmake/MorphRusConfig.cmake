find_path(MorphRus_INCLUDE_DIR
    NAMES rus.h
    PATHS /usr/local/include/libmorph /usr/include/libmorph)

find_library(MorphRus_LIBRARY
    NAMES morphrus fuzzyrus
    PATHS /usr/local/lib /usr/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MorphRus
    REQUIRED_VARS MorphRus_LIBRARY MorphRus_INCLUDE_DIR)

mark_as_advanced(MorphRus_INCLUDE_DIR MorphRus_LIBRARY)
