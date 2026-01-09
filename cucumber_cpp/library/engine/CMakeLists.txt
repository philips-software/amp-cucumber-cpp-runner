add_library(cucumber_cpp.library.engine ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.engine PRIVATE
    ExecutionContext.cpp
    ExecutionContext.hpp
    Hook.cpp
    Hook.hpp
    Step.cpp
    Step.hpp
    StringTo.hpp
)

target_include_directories(cucumber_cpp.library.engine PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.engine PUBLIC
    base64
    # cucumber_cpp.library
    cucumber_cpp.library.util
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.support
    cucumber_cpp.library.assemble
    cucumber_cpp.library.runtime
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
