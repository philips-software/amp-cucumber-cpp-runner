add_executable(cucumber_cpp.library.runtime.test)
add_test(NAME cucumber_cpp.library.runtime.test COMMAND cucumber_cpp.library.runtime.test)

target_link_libraries(cucumber_cpp.library.runtime.test PUBLIC
    cucumber_cpp.library.runtime
    cucumber_cpp.library.engine
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.runtime.test PRIVATE
    TestDummy.cpp
    TestCaseRunner.cpp
)
