add_library(cucumber_cpp.library.assemble STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.assemble PRIVATE
    AssembledTestCase.hpp
    AssembledTestSuite.hpp
    AssembleTestSuites.cpp
    AssembleTestSuites.hpp
)

target_include_directories(cucumber_cpp.library.assemble PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.assemble PUBLIC
    # cucumber_cpp.library
    # cucumber_cpp.library.util
    cucumber_cpp.library.cucumber_expression
)

if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
