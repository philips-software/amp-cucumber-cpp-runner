cmake_minimum_required(VERSION 3.24)

project(cucumber-cpp-runner LANGUAGES C CXX VERSION 1.0.0) # x-release-please-version

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

option(CCR_BUILD_TESTS "Enable building the tests" ${CCR_DEFAULTOPT})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

set_directory_properties(PROPERTY USE_FOLDERS ON)

include(FetchContent)
include(GNUInstallDirs)
include(CTest)
include(GoogleTest)

add_subdirectory(extern)
add_subdirectory(cucumber-cpp)
add_subdirectory(cucumber-cpp-runner)
add_subdirectory(cucumber-cpp-example)
