add_executable(cucumber_cpp.library.cucumber_expression.test)
add_test(NAME cucumber_cpp.library.cucumber_expression.test COMMAND cucumber_cpp.library.cucumber_expression.test)

target_link_libraries(cucumber_cpp.library.cucumber_expression.test PUBLIC
    gmock_main
    cucumber_cpp.library.cucumber_expression
)

target_sources(cucumber_cpp.library.cucumber_expression.test PRIVATE
    TestExpression.cpp
    TestExpressionParser.cpp
    TestExpressionTokenizer.cpp
)
