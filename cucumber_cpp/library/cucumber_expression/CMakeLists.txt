add_library(cucumber_cpp.library.cucumber_expression STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.cucumber_expression PRIVATE
    Argument.cpp
    Argument.hpp
    Ast.cpp
    Ast.hpp
    Errors.cpp
    Errors.hpp
    Expression.cpp
    Expression.hpp
    ExpressionParser.cpp
    ExpressionParser.hpp
    ExpressionTokenizer.cpp
    ExpressionTokenizer.hpp
    MatchRange.cpp
    MatchRange.hpp
    ParameterRegistry.cpp
    ParameterRegistry.hpp
    RegularExpression.hpp
    TreeRegexp.cpp
    TreeRegexp.hpp
)

target_include_directories(cucumber_cpp.library.cucumber_expression PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.cucumber_expression PUBLIC
    cucumber_gherkin_lib
    cucumber_cpp.library
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
