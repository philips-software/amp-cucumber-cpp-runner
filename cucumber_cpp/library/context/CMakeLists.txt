add_library(cucumber_cpp.library.context ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.context PRIVATE
    Context.cpp
    Context.hpp
)

target_include_directories(cucumber_cpp.library.context PUBLIC
    ../../..
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (CCR_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()
