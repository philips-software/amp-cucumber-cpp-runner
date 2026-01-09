add_library(cucumber_cpp.library.formatter.helper ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.formatter.helper PRIVATE
    EventDataCollector.cpp
    EventDataCollector.hpp
    GetColorFunctions.hpp
    GetColorFunctions.cpp
    GherkinDocumentParser.cpp
    GherkinDocumentParser.hpp
    IndentString.cpp
    IndentString.hpp
    IssueHelpers.cpp
    IssueHelpers.hpp
    KeywordType.cpp
    KeywordType.hpp
    LocationHelpers.cpp
    LocationHelpers.hpp
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
    cpp-terminal
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.support
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
