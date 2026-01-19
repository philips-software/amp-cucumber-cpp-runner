add_library(cucumber_cpp.library.runtime ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.runtime PRIVATE
    Coordinator.cpp
    Coordinator.hpp
    MakeRuntime.cpp
    MakeRuntime.hpp
    NestedTestCaseRunner.cpp
    NestedTestCaseRunner.hpp
    SerialRuntimeAdapter.cpp
    SerialRuntimeAdapter.hpp
    TestCaseRunner.cpp
    TestCaseRunner.hpp
    Worker.cpp
    Worker.hpp
)

target_include_directories(cucumber_cpp.library.runtime PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.runtime PUBLIC
    cucumber_cpp.library.assemble
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.support
    cucumber_cpp.library.util
    fmt-header-only
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
