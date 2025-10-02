add_executable(cucumber_cpp.library.cucumber_expression.test)
add_test(NAME cucumber_cpp.library.cucumber_expression.test COMMAND cucumber_cpp.library.cucumber_expression.test)

target_link_libraries(cucumber_cpp.library.cucumber_expression.test PUBLIC
    gmock_main
    cucumber_cpp.library.cucumber_expression
    yaml-cpp::yaml-cpp
    GTest::gmock
)

target_sources(cucumber_cpp.library.cucumber_expression.test PRIVATE
    TestExpression.cpp
    TestExpressionParser.cpp
    TestExpressionTokenizer.cpp
    TestTransformation.cpp
)

add_custom_command(
    TARGET cucumber_cpp.library.cucumber_expression.test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/testdata
        $<TARGET_FILE_DIR:cucumber_cpp.library.cucumber_expression.test>/testdata
)

add_custom_command(
    TARGET cucumber_cpp.library.cucumber_expression.test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/testdata
        $<TARGET_FILE_DIR:cucumber_cpp.library.cucumber_expression.test>/../testdata
)
