add_executable(cucumber_cpp.library.context.test)
gtest_discover_tests(
    cucumber_cpp.library.context.test
    DISCOVERY_MODE PRE_TEST
    DISCOVERY_TIMEOUT 30
)

target_link_libraries(cucumber_cpp.library.context.test PUBLIC
    cucumber_cpp.library.context
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.context.test PRIVATE
    TestContext.cpp
)
