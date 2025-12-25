add_library(cucumber_cpp.library.api STATIC ${CCR_EXCLUDE_FROM_ALL})

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
    # cucumber_cpp.library
    # cucumber_cpp.library.util
    cucumber_cpp.library.cucumber_expression
)

if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
