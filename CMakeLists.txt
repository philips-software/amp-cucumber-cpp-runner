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

set(CUCUMBER_CPP_FETCH_DEPS ${CCR_FETCH_DEPS})

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

set_directory_properties(PROPERTY USE_FOLDERS ON)

include(FetchContent)
include(GNUInstallDirs)
include(CTest)

include(cmake/dependencies.cpm.cmake)
add_subdirectory(external)

if (TARGET re2::re2)
    set(CCR_HAS_RE2 On)
else()
    set(CCR_HAS_RE2 Off)
endif()

# Sanitizer flags are set after external dependencies so that fetched third-party
# libraries (abseil, re2) are not compiled with sanitizer instrumentation.
# Abseil's constexpr function-pointer comparisons are not compatible with GCC's
# ASan/UBSan instrumentation, which causes build failures.
if (PROJECT_IS_TOP_LEVEL AND NOT CCR_ENABLE_COVERAGE)
    if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_SIMULATE_ID MATCHES "MSVC"))
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_COMPILE_WARNING_AS_ERROR On)
endif()

add_subdirectory(cucumber_cpp)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(compatibility)
endif()
