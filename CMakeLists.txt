cmake_minimum_required(VERSION 3.24)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(cucumber-cpp-runner LANGUAGES C CXX VERSION 1.0.0) # x-release-please-version

include(ccr_test_helpers)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CCR_STANDALONE On)
endif()

if (CCR_STANDALONE)
    set(CCR_DEFAULTOPT On)
else()
    set(CCR_DEFAULTOPT Off)
endif()

if (CCR_STANDALONE)
    set(CCR_EXCLUDE_FROM_ALL "")
else()
    set(CCR_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

option(
      CCR_FETCH_DEPS
      "Fetch dependencies via FetchContent."
      On
)

option(CCR_BUILD_TESTS "Enable building the tests" ${CCR_DEFAULTOPT})

if (CCR_BUILD_TESTS)
    ccr_enable_testing()
endif()

set(CMAKE_CXX_STANDARD 23)
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
endif()

add_subdirectory(cucumber_cpp)
