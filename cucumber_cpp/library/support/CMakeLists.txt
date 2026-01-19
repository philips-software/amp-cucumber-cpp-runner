add_library(cucumber_cpp.library.support ${CCR_EXCLUDE_FROM_ALL})

if (CCR_STANDALONE)
    target_compile_definitions(cucumber_cpp.library.support PRIVATE
        CCR_STANDALONE
    )
endif()

target_sources(cucumber_cpp.library.support PRIVATE
    Body.cpp
    Body.hpp
    HookRegistry.cpp
    HookRegistry.hpp
    Join.cpp
    Join.hpp
    StepRegistry.cpp
    StepRegistry.hpp
    StepType.hpp
    SupportCodeLibrary.cpp
    SupportCodeLibrary.hpp
    Types.hpp
)

target_include_directories(cucumber_cpp.library.support PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.support PUBLIC
    GTest::gtest
    GTest::gmock
    cucumber_cpp.library.cucumber_expression
    cucumber_cpp.library.tag_expression
    cucumber_cpp.library.util
    fmt-header-only
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
