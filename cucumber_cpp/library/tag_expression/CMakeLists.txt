set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber_cpp.library.tag_expression STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.tag_expression PRIVATE
    TagExpressionError.cpp
    TagExpressionError.hpp
)

target_include_directories(cucumber_cpp.library.tag_expression PUBLIC
    ../../..
)

# target_link_libraries(cucumber_cpp.library.tag_expression PUBLIC
# )

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
