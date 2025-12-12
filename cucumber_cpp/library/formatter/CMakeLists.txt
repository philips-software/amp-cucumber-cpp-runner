add_library(cucumber_cpp.library.formatter STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.formatter PRIVATE
    Formatter.cpp
    Formatter.hpp
    GetColorFunctions.hpp
    GetColorFunctions.cpp
    PrettyPrinter.cpp
    PrettyPrinter.hpp
    SummaryFormatter.cpp
    SummaryFormatter.hpp
)

target_include_directories(cucumber_cpp.library.formatter PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.formatter PUBLIC
    # cucumber_cpp.library
    # cucumber_cpp.library.util
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.formatter.helper
    cpp-terminal
)

add_subdirectory(helper)


if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
