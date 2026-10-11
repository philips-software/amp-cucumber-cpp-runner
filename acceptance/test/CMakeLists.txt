add_executable(acceptance.util.subprocess_test_child ${CCR_EXCLUDE_FROM_ALL})

target_sources(acceptance.util.subprocess_test_child PRIVATE
    SubprocessChild.cpp
)

add_executable(acceptance.util.test ${CCR_EXCLUDE_FROM_ALL})

target_link_libraries(acceptance.util.test PRIVATE
    acceptance.util
    GTest::gmock
    GTest::gmock_main
)

target_compile_definitions(acceptance.util.test PRIVATE
    SUBPROCESS_TEST_CHILD="$<TARGET_FILE:acceptance.util.subprocess_test_child>"
)

target_sources(acceptance.util.test PRIVATE
    TestSubprocess.cpp
)

if (CCR_BUILD_TESTS)
    add_test(NAME acceptance.util.test COMMAND acceptance.util.test)
endif()
