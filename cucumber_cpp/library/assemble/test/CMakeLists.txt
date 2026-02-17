add_executable(cucumber_cpp.library.assemble.test)
add_test(NAME cucumber_cpp.library.assemble.test COMMAND cucumber_cpp.library.assemble.test)

target_link_libraries(cucumber_cpp.library.assemble.test PUBLIC
    cucumber_cpp.library.assemble
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.assemble.test PRIVATE
    TestDummy.cpp
)
