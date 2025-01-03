add_library(cucumber_cpp.library.engine STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.engine PRIVATE
    ContextManager.cpp
    ContextManager.hpp
    FeatureFactory.cpp
    FeatureFactory.hpp
    FeatureInfo.cpp
    FeatureInfo.hpp
    HookExecutor.cpp
    HookExecutor.hpp
    RuleInfo.cpp
    RuleInfo.hpp
    ScenarioInfo.cpp
    ScenarioInfo.hpp
    Step.cpp
    Step.hpp
    StepInfo.cpp
    StepInfo.hpp
    StepType.hpp
    StringTo.hpp
    Table.cpp
    Table.hpp
    TestExecution.cpp
    TestExecution.hpp
    TestFailureHandler.cpp
    TestFailureHandler.hpp
    TestRunner.cpp
    TestRunner.hpp
)

target_include_directories(cucumber_cpp.library.engine PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.engine PUBLIC
    cucumber_cpp.library
    cucumber_cpp.library.util
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    add_subdirectory(test_helper)
endif()
