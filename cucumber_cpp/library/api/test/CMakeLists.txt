add_executable(cucumber_cpp.library.api.test)
add_test(NAME cucumber_cpp.library.api.test COMMAND cucumber_cpp.library.api.test)

target_link_libraries(cucumber_cpp.library.api.test PUBLIC
    cucumber_cpp.library.api
    GTest::gmock
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.api.test PRIVATE
    TestDummy.cpp
)
