# Find module for the header-only tobiaslocker/base64 library, which ships no CMake package config.
#
# Searches for <prefix>/include/base64.hpp in the default CMake search locations
# (e.g. /usr/local, CMAKE_PREFIX_PATH). Override with -Dbase64_INCLUDE_DIR=<dir>.
#
# Provides the imported target base64::base64.

find_path(base64_INCLUDE_DIR NAMES base64.hpp)
mark_as_advanced(base64_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(base64 REQUIRED_VARS base64_INCLUDE_DIR)

if(base64_FOUND AND NOT TARGET base64::base64)
    add_library(base64::base64 INTERFACE IMPORTED)
    set_target_properties(base64::base64 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${base64_INCLUDE_DIR}"
    )
endif()
