add_library(cucumber_cpp.library.cucumber_expression STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.cucumber_expression PRIVATE
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
    ParameterRegistry.cpp
    ParameterRegistry.hpp
)

target_include_directories(cucumber_cpp.library.cucumber_expression PUBLIC
    ../../..
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
