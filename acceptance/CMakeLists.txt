set(CMAKE_COMPILE_WARNING_AS_ERROR On)

include(${PROJECT_SOURCE_DIR}/cmake/ccr_add_plugin.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/cucumber_discover_tests.cmake)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sandbox)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sandbox/Sandbox.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/sandbox/Sandbox.cpp COPYONLY)

ccr_add_plugin(acceptance.sandbox ${CMAKE_CURRENT_BINARY_DIR}/sandbox/Sandbox.cpp)
set_target_properties(acceptance.sandbox PROPERTIES
    EXCLUDE_FROM_ALL ON
)

# Export compiler command line configuration for on-the-fly plugin compilation in acceptance tests
set(CCR_ACCEPTANCE_SANDBOX_RSP "${CMAKE_CURRENT_BINARY_DIR}/sandbox/compile_flags.rsp")
file(GENERATE
    OUTPUT "${CCR_ACCEPTANCE_SANDBOX_RSP}"
    CONTENT "$<JOIN:$<TARGET_PROPERTY:acceptance.sandbox,COMPILE_OPTIONS>,
>
$<JOIN:$<LIST:TRANSFORM,$<TARGET_PROPERTY:acceptance.sandbox,COMPILE_DEFINITIONS>,PREPEND,-D>,
>
$<JOIN:$<LIST:TRANSFORM,$<TARGET_PROPERTY:acceptance.sandbox,INCLUDE_DIRECTORIES>,PREPEND,-isystem >,
>
"
    # file(GENERATE) evaluates once per enabled language; only the C++ flags are relevant
    CONDITION "$<COMPILE_LANGUAGE:CXX>"
)

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
    )
endif()
