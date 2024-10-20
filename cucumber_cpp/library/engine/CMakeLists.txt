add_library(cucumber_cpp.library.engine STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.engine PRIVATE
    ContextManager.cpp
    ContextManager.hpp
    Events.cpp
    Events.hpp
    FeatureFactory.cpp
    FeatureFactory.hpp
    FeatureInfo.cpp
    FeatureInfo.hpp
    HookExecutor.cpp
    HookExecutor.hpp
    Observer.hpp
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
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    add_subdirectory(test_helper)
endif()
