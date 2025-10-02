add_executable(cucumber_cpp.library.tag_expression.test)
add_test(NAME cucumber_cpp.library.tag_expression.test COMMAND cucumber_cpp.library.tag_expression.test)

target_link_libraries(cucumber_cpp.library.tag_expression.test PUBLIC
    gmock_main
    cucumber_cpp.library.tag_expression
    yaml-cpp::yaml-cpp
    GTest::gmock
)

target_sources(cucumber_cpp.library.tag_expression.test PRIVATE
    TestErrors.cpp
    TestEvaluations.cpp
    TestParsing.cpp
    TestToken.cpp
)

add_custom_command(
    TARGET cucumber_cpp.library.tag_expression.test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/testdata $<TARGET_FILE_DIR:cucumber_cpp.library.tag_expression.test>/testdata
)

add_custom_command(
    TARGET cucumber_cpp.library.tag_expression.test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/testdata $<TARGET_FILE_DIR:cucumber_cpp.library.tag_expression.test>/../testdata
)
