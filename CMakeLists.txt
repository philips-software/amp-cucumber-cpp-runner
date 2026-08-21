cmake_minimum_required(VERSION 3.24)
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(cucumber-cpp-runner LANGUAGES C CXX VERSION 4.1.0) # x-release-please-version

include(cmake/ccr_test_helpers.cmake)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CCR_STANDALONE On)
endif()

if (CCR_STANDALONE)
    set(CCR_DEFAULTOPT On)
    set(CCR_EXCLUDE_FROM_ALL "")
else()
    set(CCR_DEFAULTOPT Off)
    set(CCR_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

option(CCR_FETCH_DEPS "Fetch dependencies via FetchContent." ${CCR_DEFAULTOPT} )
option(CCR_BUILD_TESTS "Enable build of the tests" ${CCR_DEFAULTOPT})
option(CCR_USE_RE2 "Use the RE2 regex engine (fetched when CCR_FETCH_DEPS is set, otherwise auto-detected)" ${CCR_DEFAULTOPT})
option(CCR_ENABLE_COVERAGE "Enable compiler flags for code coverage measurements" Off)
option(CCR_ENABLE_TIME_PROFILE "Enable compiler flags for time profiling measurements" Off)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CCR_BUILD_TESTS)
    ccr_enable_testing()
endif()

if (CCR_ENABLE_COVERAGE)
    find_program(GCOVR_PATH gcovr)

    if (NOT GCOVR_PATH)
    message(FATAL_ERROR "Could not find gcovr, which is required for code coverage IDE integration")
    endif()

    add_custom_target(
        generate-coverage-report
        COMMAND ${GCOVR_PATH} --delete --lcov ${CMAKE_CURRENT_BINARY_DIR}/lcov.info
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating code coverage report in LCOV format"
    )
endif()

if (CCR_ENABLE_TIME_PROFILE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-ftime-trace)
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

# Compiler cache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
else()
    find_program(SCCACHE_PROGRAM sccache)
    if(SCCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
        message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
    endif()
endif()

set_directory_properties(PROPERTY USE_FOLDERS ON)

include(FetchContent)
include(GNUInstallDirs)
include(CTest)

# Enable sanitizers globally, before any dependency is added, so every target
# (including fetched third-party libraries) is instrumented. Consistent
# instrumentation keeps libc++ container annotations coherent across module
# boundaries; mixing instrumented and uninstrumented code produces false-positive
# container-overflow reports.
if (PROJECT_IS_TOP_LEVEL AND NOT CCR_ENABLE_COVERAGE
        AND NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_SIMULATE_ID MATCHES "MSVC"))
    set(CCR_ENABLE_SANITIZERS On)
endif()

if (CCR_ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

include(cmake/dependencies.cpm.cmake)
add_subdirectory(external)

if (TARGET re2::re2)
    set(CCR_HAS_RE2 On)
else()
    set(CCR_HAS_RE2 Off)
endif()

# Opt specific dependencies out of sanitizer instrumentation because of known
# limitations: abseil's constexpr function-pointer comparisons are not compatible
# with GCC's ASan/UBSan instrumentation, which causes build failures. Later target
# options override the globally inherited flags.
if (CCR_ENABLE_SANITIZERS)
    function(ccr_disable_sanitizers dir)
        get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
        foreach(target IN LISTS targets)
            get_target_property(type ${target} TYPE)
            if (NOT type STREQUAL "INTERFACE_LIBRARY" AND NOT type STREQUAL "UTILITY")
                target_compile_options(${target} PRIVATE -fno-sanitize=address -fno-sanitize=undefined)
                target_link_options(${target} PRIVATE -fno-sanitize=address -fno-sanitize=undefined)
            endif()
        endforeach()

        get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
        foreach(subdir IN LISTS subdirs)
            ccr_disable_sanitizers("${subdir}")
        endforeach()
    endfunction()

    foreach(dep_dir IN ITEMS "${abseil-cpp_SOURCE_DIR}" "${re2_SOURCE_DIR}")
        if (dep_dir AND IS_DIRECTORY "${dep_dir}")
            ccr_disable_sanitizers("${dep_dir}")
        endif()
    endforeach()
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_COMPILE_WARNING_AS_ERROR On)
endif()

add_subdirectory(cucumber_cpp)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(compatibility)
endif()
