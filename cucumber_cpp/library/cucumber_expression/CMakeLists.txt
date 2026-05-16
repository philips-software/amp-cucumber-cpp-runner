add_library(cucumber_cpp.library.cucumber_expression ${CCR_EXCLUDE_FROM_ALL})

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
    Group.cpp
    Group.hpp
    MatchRange.cpp
    MatchRange.hpp
    ParameterRegistry.cpp
    ParameterRegistry.hpp
    Re2RegexStrategy.cpp
    Re2RegexStrategy.hpp
    RegexStrategy.hpp
    RegularExpression.cpp
    RegularExpression.hpp
    StdRegexStrategy.cpp
    StdRegexStrategy.hpp
    TreeRegexp.cpp
    TreeRegexp.hpp
)

target_include_directories(cucumber_cpp.library.cucumber_expression PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.cucumber_expression PUBLIC
    fmt::fmt
    re2::re2
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
