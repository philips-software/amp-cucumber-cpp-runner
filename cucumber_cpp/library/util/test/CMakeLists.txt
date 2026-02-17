add_executable(cucumber_cpp.library.util.test)
add_test(NAME cucumber_cpp.library.util.test COMMAND cucumber_cpp.library.util.test)

target_link_libraries(cucumber_cpp.library.util.test PUBLIC
    cucumber_cpp.library.util
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.util.test PRIVATE
    TestDummy.cpp
)
