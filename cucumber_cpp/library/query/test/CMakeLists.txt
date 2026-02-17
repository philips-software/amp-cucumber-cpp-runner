add_executable(cucumber_cpp.library.query.test)
add_test(NAME cucumber_cpp.library.query.test COMMAND cucumber_cpp.library.query.test)

target_link_libraries(cucumber_cpp.library.query.test PUBLIC
    cucumber_cpp.library.query
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.query.test PRIVATE
    TestDummy.cpp
)
