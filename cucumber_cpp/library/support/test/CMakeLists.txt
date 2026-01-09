add_executable(cucumber_cpp.library.support.test)
add_test(NAME cucumber_cpp.library.support.test COMMAND cucumber_cpp.library.support.test)

target_link_libraries(cucumber_cpp.library.support.test PUBLIC
    gmock_main
    cucumber_cpp.library.support
    GTest::gmock
)

target_sources(cucumber_cpp.library.support.test PRIVATE
    TestDummy.cpp
)
