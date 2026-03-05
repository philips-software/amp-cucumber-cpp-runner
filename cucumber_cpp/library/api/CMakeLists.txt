add_library(cucumber_cpp.library.api ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.api PRIVATE
    Formatters.cpp
    Formatters.hpp
    Gherkin.cpp
    Gherkin.hpp
    RunCucumber.cpp
    RunCucumber.hpp
)

target_include_directories(cucumber_cpp.library.api PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.api PUBLIC
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.formatter
    cucumber_cpp.library.runtime
    cucumber_cpp.library.support
    cucumber_cpp.library.util
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
