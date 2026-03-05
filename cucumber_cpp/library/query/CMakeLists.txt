add_library(cucumber_cpp.library.query ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.query PRIVATE
    Query.cpp
    Query.hpp
)

target_include_directories(cucumber_cpp.library.query PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.query PUBLIC
    cucumber_gherkin_lib
    cucumber_cpp.library.util
    fmt::fmt
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
