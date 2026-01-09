add_library(cucumber_cpp.library.support ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.support PRIVATE
    Body.cpp
    Body.hpp
    Duration.cpp
    Duration.hpp
    HookRegistry.cpp
    HookRegistry.hpp
    Join.cpp
    Join.hpp
    StepRegistry.cpp
    StepRegistry.hpp
    SupportCodeLibrary.cpp
    SupportCodeLibrary.hpp
    Timestamp.cpp
    Timestamp.hpp
    Types.hpp

)

target_include_directories(cucumber_cpp.library.support PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.support PUBLIC
    GTest::gtest
    GTest::gmock
    cucumber_cpp.library.util
)

if (CCR_BUILD_TESTS)
    # add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
