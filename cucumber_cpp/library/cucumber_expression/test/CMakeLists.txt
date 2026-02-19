add_executable(cucumber_cpp.library.cucumber_expression.test)
add_test(NAME cucumber_cpp.library.cucumber_expression.test COMMAND cucumber_cpp.library.cucumber_expression.test)

target_link_libraries(cucumber_cpp.library.cucumber_expression.test PUBLIC
    cucumber_cpp.library.cucumber_expression
    GTest::gmock
    GTest::gmock_main
    yaml-cpp::yaml-cpp
)

target_sources(cucumber_cpp.library.cucumber_expression.test PRIVATE
    TestExpression.cpp
    TestExpressionParser.cpp
    TestExpressionTokenizer.cpp
    TestRegularExpression.cpp
    TestTransformation.cpp
    TestTreeRegexp.cpp
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
