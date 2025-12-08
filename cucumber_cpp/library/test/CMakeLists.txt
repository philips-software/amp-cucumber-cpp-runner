add_executable(cucumber_cpp.library.test)
add_test(NAME cucumber_cpp.library.test COMMAND cucumber_cpp.library.test)

target_link_libraries(cucumber_cpp.library.test PUBLIC
    cucumber_cpp.library
    GTest::gtest
    GTest::gmock
    GTest::gmock_main
    cucumber_cpp.library.engine.test_helper
    cucumber_cpp.library.engine.test_helper.steps
)

target_sources(cucumber_cpp.library.test PRIVATE
    TestApplication.cpp
    TestContext.cpp
    TestSteps.cpp
)
