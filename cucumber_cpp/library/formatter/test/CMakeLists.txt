add_executable(cucumber_cpp.library.formatter.test)
add_test(NAME cucumber_cpp.library.formatter.test COMMAND cucumber_cpp.library.formatter.test)

target_link_libraries(cucumber_cpp.library.formatter.test PUBLIC
    cucumber_cpp.library.formatter
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.formatter.test PRIVATE
    TestDummy.cpp
)
