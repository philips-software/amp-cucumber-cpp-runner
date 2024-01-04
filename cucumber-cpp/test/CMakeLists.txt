add_executable(cucumber-cpp.test)
add_test(NAME cucumber-cpp.test COMMAND cucumber-cpp.test)

target_link_libraries(cucumber-cpp.test
    PUBLIC
        cucumber-cpp

        gtest
        gmock
        gmock_main
)

target_sources(cucumber-cpp.test PRIVATE
    TestContext.cpp
    TestTagExpression.cpp
    TestSteps.cpp
)
