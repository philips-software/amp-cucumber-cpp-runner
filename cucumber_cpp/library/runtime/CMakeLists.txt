add_library(cucumber_cpp.library.runtime STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.runtime PRIVATE
    Coordinator.cpp
    Coordinator.hpp
    MakeRuntime.cpp
    MakeRuntime.hpp
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
    # cucumber_cpp.library
    # cucumber_cpp.library.util
    cucumber_cpp.library.cucumber_expression
)

if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
