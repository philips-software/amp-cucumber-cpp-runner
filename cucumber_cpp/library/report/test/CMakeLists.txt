add_executable(cucumber_cpp.library.report.test)
add_test(NAME cucumber_cpp.library.report.test COMMAND cucumber_cpp.library.report.test)

target_link_libraries(cucumber_cpp.library.report.test PUBLIC
    GTest::gtest
    GTest::gmock
    GTest::gmock_main

    cucumber_cpp.library.report
    cucumber_cpp.library.engine.test_helper.steps
)

target_sources(cucumber_cpp.library.report.test PRIVATE
    TestStdOutReport.cpp
)
