add_executable(cucumber_cpp.library.formatter.helper.test)
add_test(NAME cucumber_cpp.library.formatter.helper.test COMMAND cucumber_cpp.library.formatter.helper.test)

target_link_libraries(cucumber_cpp.library.formatter.helper.test PUBLIC
    cucumber_cpp.library.formatter.helper
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.formatter.helper.test PRIVATE
    TestDummy.cpp
)
