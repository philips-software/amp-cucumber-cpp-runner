add_library(cucumber_cpp.library.util ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.util PRIVATE
    ArgumentGroupToMessageGroup.cpp
    ArgumentGroupToMessageGroup.hpp
    Body.cpp
    Body.hpp
    Broadcaster.cpp
    Broadcaster.hpp
    DocString.hpp
    Duration.cpp
    Duration.hpp
    GetWorstTestStepResult.cpp
    GetWorstTestStepResult.hpp
    HookData.hpp
    HookFactory.hpp
    Immoveable.hpp
    NestedTestCaseRunnerError.hpp
    StepFactory.hpp
    StepOrHookStarted.hpp
    Table.hpp
    TestDuration.hpp
    TestException.hpp
    TestRunHookStarted.hpp
    TestStepResult.hpp
    TestStepResultStatus.hpp
    TestStepStarted.hpp
    Timestamp.cpp
    Timestamp.hpp
    ToLower.hpp
    TransformDocString.cpp
    TransformDocString.hpp
    TransformHookData.cpp
    TransformHookData.hpp
    TransformPickleTag.cpp
    TransformPickleTag.hpp
    TransformTable.cpp
    TransformTable.hpp
    TransformTag.cpp
    TransformTag.hpp
    TransformTestRunHookStarted.cpp
    TransformTestRunHookStarted.hpp
    TransformTestStepResult.cpp
    TransformTestStepResult.hpp
    TransformTestStepResultStatus.cpp
    TransformTestStepResultStatus.hpp
    TransformTestStepStarted.cpp
    TransformTestStepStarted.hpp
    Trim.hpp
)

target_include_directories(cucumber_cpp.library.util PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.util PUBLIC
    cucumber_gherkin_lib
    fmt::fmt
    GTest::gtest
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
