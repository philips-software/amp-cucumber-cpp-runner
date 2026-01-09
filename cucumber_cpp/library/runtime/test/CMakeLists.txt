add_executable(cucumber_cpp.library.runtime.test)
add_test(NAME cucumber_cpp.library.runtime.test COMMAND cucumber_cpp.library.runtime.test)

target_link_libraries(cucumber_cpp.library.runtime.test PUBLIC
    gmock_main
    cucumber_cpp.library.runtime
    GTest::gmock
)

target_sources(cucumber_cpp.library.runtime.test PRIVATE
    TestDummy.cpp
)
