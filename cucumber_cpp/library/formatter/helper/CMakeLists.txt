add_library(cucumber_cpp.library.formatter.helper ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.formatter.helper PRIVATE
    FormatMessages.cpp
    FormatMessages.hpp
    IndentString.cpp
    IndentString.hpp
    PrintMessages.cpp
    PrintMessages.hpp
    TextBuilder.cpp
    TextBuilder.hpp
    Theme.cpp
    Theme.hpp
)

target_include_directories(cucumber_cpp.library.formatter.helper PUBLIC
    ../../../..
)

target_link_libraries(cucumber_cpp.library.formatter.helper PUBLIC
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.support
    cucumber_cpp.library.query
    fmt::fmt
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
