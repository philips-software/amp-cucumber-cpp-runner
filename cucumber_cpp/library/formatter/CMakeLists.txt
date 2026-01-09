add_library(cucumber_cpp.library.formatter ${CCR_EXCLUDE_FROM_ALL})

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
    cpp-terminal
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.formatter.helper
    cucumber_cpp.library.query
    cucumber_cpp.library.support
    cucumber_cpp.library.util
    nlohmann_json
)

add_subdirectory(helper)


if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
