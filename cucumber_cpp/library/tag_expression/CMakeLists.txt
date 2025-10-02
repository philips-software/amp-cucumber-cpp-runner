set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber_cpp.library.tag_expression STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.tag_expression PRIVATE
    Model.cpp
    Model.hpp
    TagExpressionError.cpp
    TagExpressionError.hpp
    TagExpressionParser.cpp
    TagExpressionParser.hpp
    Token.cpp
    Token.hpp
)

target_include_directories(cucumber_cpp.library.tag_expression PUBLIC
    ../../..
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
