cmake_minimum_required(VERSION 3.24)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(cucumber-cpp-runner LANGUAGES C CXX VERSION 4.0.0) # x-release-please-version

include(ccr_test_helpers)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CCR_STANDALONE On)
endif()

if (CCR_STANDALONE)
    set(CCR_DEFAULTOPT On)
    set(CCR_EXCLUDE_FROM_ALL "")
    set(CMAKE_COMPILE_WARNING_AS_ERROR On)
else()
    set(CCR_DEFAULTOPT Off)
    set(CCR_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

option(CCR_FETCH_DEPS "Fetch dependencies via FetchContent." ${CCR_DEFAULTOPT} )
option(CCR_BUILD_TESTS "Enable build of the tests" ${CCR_DEFAULTOPT})
option(CCR_ENABLE_COVERAGE "Enable compiler flags for code coverage measurements" Off)
option(CCR_ENABLE_TIME_PROFILE "Enable compiler flags for time profiling measurements" Off)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CCR_STANDALONE AND NOT CCR_ENABLE_COVERAGE)
    if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_SIMULATE_ID MATCHES "MSVC"))
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

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

if (CCR_FETCH_DEPS)
    add_subdirectory(external)
else()
    find_package(CLI11 REQUIRED)
    find_package(nlohmann_json REQUIRED)
    find_package(GTest REQUIRED)
    find_package(pugixml REQUIRED)
    find_package(cucumber_messages REQUIRED)
    find_package(cucumber_gherkin REQUIRED)
    find_package(fmt REQUIRED)

    if (CCR_BUILD_TESTS)
        find_package(yaml-cpp REQUIRED)
    endif()
endif()

add_subdirectory(cucumber_cpp)

if (CCR_STANDALONE)
    add_subdirectory(compatibility)
endif()
