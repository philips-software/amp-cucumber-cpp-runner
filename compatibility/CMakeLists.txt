include(${PROJECT_SOURCE_DIR}/cmake/ccr_add_plugin.cmake)

add_library(compatibility.base)
target_sources(compatibility.base PRIVATE
    BaseCompatibility.cpp
    BaseCompatibility.hpp
)

target_link_libraries(compatibility.base PUBLIC
    cucumber_cpp
    GTest::gtest
)

target_include_directories(compatibility.base PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

file(TO_CMAKE_PATH "${CMAKE_BINARY_DIR}" CCR_BINARY_DIR_CMAKE)
target_compile_definitions(compatibility.base PRIVATE
    CCR_BINARY_DIR="${CCR_BINARY_DIR_CMAKE}"
)

# Build a plugin for each kit that has a .cpp file
file(GLOB kits RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
foreach(kit ${kits})
    if (IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${kit} AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/${kit}/${kit}.cpp)
        ccr_add_plugin(compatibility.${kit}.plugin ${kit}/${kit}.cpp)
    endif()
endforeach()

# Single test executable that loads plugins at runtime
add_executable(compatibility.test)
target_sources(compatibility.test PRIVATE
    TestCompatibility.cpp
)
target_link_libraries(compatibility.test PRIVATE
    GTest::gtest_main
    compatibility.base
)

# Plugins resolve symbols from this executable at runtime via dlopen.
# ENABLE_EXPORTS creates the dynamic symbol table, but the linker
# normally strips unreferenced symbols from static libraries.
# WHOLE_ARCHIVE ensures all cucumber_cpp symbols are linked and exported
# for plugin use.
if (NOT WIN32)
    target_link_libraries(compatibility.test PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,cucumber_cpp.library,cucumber_cpp.library.api,cucumber_cpp.library.engine,cucumber_cpp.library.support,cucumber_cpp.library.util,GTest::gtest,GTest::gmock>
    )
endif()

set_target_properties(compatibility.test PROPERTIES
    ENABLE_EXPORTS ON
)

file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" COMPAT_SOURCE_DIR_CMAKE)
file(TO_CMAKE_PATH "${CMAKE_BINARY_DIR}" COMPAT_BUILD_DIR_CMAKE)

target_compile_definitions(compatibility.test PRIVATE
    COMPAT_SOURCE_DIR="${COMPAT_SOURCE_DIR_CMAKE}"
    COMPAT_BUILD_DIR="${COMPAT_BUILD_DIR_CMAKE}"
    COMPAT_PLUGIN_DIR="$<TARGET_FILE_DIR:compatibility.test>"
)

if(CCR_BUILD_TESTS)
    if(NOT CMAKE_CROSSCOMPILING AND NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        gtest_discover_tests(compatibility.test DISCOVERY_MODE PRE_TEST)
    else()
        add_test(NAME compatibility.test COMMAND compatibility.test)
    endif()
endif()
