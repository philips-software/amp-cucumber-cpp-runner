set(CMAKE_COMPILE_WARNING_AS_ERROR On)

include(${PROJECT_SOURCE_DIR}/cmake/ccr_add_plugin.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/cucumber_discover_tests.cmake)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sandbox)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sandbox/Sandbox.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/sandbox/Sandbox.cpp COPYONLY)

ccr_add_plugin(acceptance.sandbox ${CMAKE_CURRENT_BINARY_DIR}/sandbox/Sandbox.cpp)
set_target_properties(acceptance.sandbox PROPERTIES
    EXCLUDE_FROM_ALL ON
)

# Host executable built from a scenario provided source containing main().
# Like cucumber_cpp.cli it exports all cucumber_cpp symbols, so plugins built by
# acceptance.sandbox can resolve them from the host at runtime.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sandbox/SandboxHost.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/sandbox/SandboxHost.cpp COPYONLY)

add_executable(acceptance.sandbox_host EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/sandbox/SandboxHost.cpp)

target_link_libraries(acceptance.sandbox_host PRIVATE
    cucumber_cpp
)

set_target_properties(acceptance.sandbox_host PROPERTIES
    ENABLE_EXPORTS ON
)

if (NOT WIN32)
    target_link_libraries(acceptance.sandbox_host PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,cucumber_cpp.library,cucumber_cpp.library.api,cucumber_cpp.library.engine,cucumber_cpp.library.support,cucumber_cpp.library.util,GTest::gtest,GTest::gmock>
    )
endif()

add_subdirectory(util)
add_subdirectory(steps)
add_subdirectory(test)

add_executable(acceptance.test ${CCR_EXCLUDE_FROM_ALL})

target_link_libraries(acceptance.test PRIVATE
    cucumber_cpp.runner
    acceptance.steps
)

add_dependencies(acceptance.test cucumber_cpp.cli)

if (CCR_BUILD_TESTS)
    cucumber_discover_tests(acceptance.test
        EXTRA_ARGS --format summary -- ${CMAKE_CURRENT_LIST_DIR}/features
        DISCOVERY_MODE PRE_TEST
        PROPERTIES LABELS acceptance
    )
endif()
