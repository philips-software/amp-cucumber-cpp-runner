cmake_minimum_required(VERSION 3.24)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(cucumber-cpp-runner LANGUAGES C CXX VERSION 3.1.1) # x-release-please-version

include(ccr_test_helpers)

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

option(CCR_FETCH_DEPS "Fetch dependencies via FetchContent." On )
option(CCR_BUILD_TESTS "Enable build of the tests" ${CCR_DEFAULTOPT})
option(CCR_ENABLE_COVERAGE "Enable compiler flags for code coverage measurements" Off)

set(BUILD_SHARED_LIBS  Off CACHE STRING "")

message(STATUS "CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CXX_SIMULATE_ID: ${CMAKE_CXX_SIMULATE_ID}")

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND NOT CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

set_directory_properties(PROPERTY USE_FOLDERS ON)

include(FetchContent)
include(GNUInstallDirs)
include(CTest)
include(GoogleTest)

if (CCR_FETCH_DEPS)
    add_subdirectory(external)
else()
    find_package(CLI11 REQUIRED)
    find_package(nlohmann_json REQUIRED)
    find_package(GTest REQUIRED)
    find_package(pugixml REQUIRED)
    find_package(cucumber_messages REQUIRED)
    find_package(cucumber_gherkin REQUIRED)
    find_package(yaml-cpp REQUIRED)
endif()

add_subdirectory(cucumber_cpp)
add_subdirectory(compatibility)
