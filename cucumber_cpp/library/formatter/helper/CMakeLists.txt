add_library(cucumber_cpp.library.formatter.helper STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.formatter.helper PRIVATE
    EventDataCollector.cpp
    EventDataCollector.hpp
    GherkinDocumentParser.cpp
    GherkinDocumentParser.hpp
    IndentString.cpp
    IndentString.hpp
    IssueHelpers.cpp
    IssueHelpers.hpp
    KeywordType.cpp
    KeywordType.hpp
    PickleParser.cpp
    PickleParser.hpp
    SummaryHelpers.hpp
    SummaryHelpers.cpp
    TestCaseAttemptFormatter.cpp
    TestCaseAttemptFormatter.hpp
    TestCaseAttemptParser.cpp
    TestCaseAttemptParser.hpp
)

target_include_directories(cucumber_cpp.library.formatter.helper PUBLIC
    ../../../..
)

target_link_libraries(cucumber_cpp.library.formatter.helper PUBLIC
    # cucumber_cpp.library
    # cucumber_cpp.library.util
    cucumber_cpp.library.cucumber_expression
)

if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
