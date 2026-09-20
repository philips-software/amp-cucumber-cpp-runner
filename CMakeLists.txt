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

option(CCR_ENABLE_COVERAGE "Enable compiler flags for code coverage measurements" Off)
option(CCR_ENABLE_SANITIZERS "Enable compiler flags for the address and undefined behaviour sanitizers" Off)
option(CCR_ENABLE_TIME_PROFILE "Enable compiler flags for time profiling measurements" Off)

option(CCR_USE_RE2 "Use the RE2 regex engine (fetched when CCR_FETCH_DEPS is set, otherwise auto-detected)" ${CCR_DEFAULTOPT})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CCR_ENABLE_SANITIZERS AND CCR_ENABLE_COVERAGE)
    message(FATAL_ERROR "CCR_ENABLE_SANITIZERS and CCR_ENABLE_COVERAGE are mutually exclusive.")
endif()

if(CCR_ENABLE_COVERAGE AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(WARNING "Code coverage is only enabled with the GNU compiler.")
endif()

if (CCR_BUILD_TESTS)
    ccr_enable_testing()
endif()

if (CCR_ENABLE_COVERAGE)
    find_program(GCOVR_PROGRAM gcovr REQUIRED)

    add_custom_target(
        generate-coverage-report
        COMMAND ${GCOVR_PROGRAM} --version
        COMMAND ${GCOVR_PROGRAM}
            --root ${CMAKE_CURRENT_SOURCE_DIR}
            --merge-lines --delete --exclude-unreachable-branches --exclude-throw-branches --exclude-noncode-lines
            --filter cucumber_cpp
            --exclude=.*/example/.* --exclude=.*/external/.* --exclude=.*/test/.* --exclude=.*/test_helper/.*
            --exclude-lines-by-pattern '.*assert\(.*\);|.*really_assert\(.*\);|.*std::abort();'
            --txt-summary
            --lcov ${CMAKE_CURRENT_BINARY_DIR}/lcov.info
            --cobertura ${CMAKE_CURRENT_BINARY_DIR}/cobertura.xml --cobertura-pretty
            --sonarqube ${CMAKE_CURRENT_BINARY_DIR}/sonarqube.xml
            --markdown-summary ${CMAKE_CURRENT_BINARY_DIR}/coverage-summary.md
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating code coverage reports"
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

include(cmake/dependencies.cpm.cmake)
add_subdirectory(external)

if (CCR_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

if (TARGET re2::re2)
    set(CCR_HAS_RE2 On)
else()
    set(CCR_HAS_RE2 Off)
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_COMPILE_WARNING_AS_ERROR On)
endif()

add_subdirectory(cucumber_cpp)

if (CCR_BUILD_TESTS)
    add_subdirectory(compatibility)
endif()
