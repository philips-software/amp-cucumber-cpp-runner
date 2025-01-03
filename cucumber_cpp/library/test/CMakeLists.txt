add_executable(cucumber_cpp.library.test)
add_test(NAME cucumber_cpp.library.test COMMAND cucumber_cpp.library.test)

target_link_libraries(cucumber_cpp.library.test PUBLIC
    cucumber_cpp.library
    gtest
    gmock
    gmock_main
)

target_sources(cucumber_cpp.library.test PRIVATE
    TestContext.cpp
    TestTagExpression.cpp
    TestSteps.cpp
)
