add_library(cucumber_cpp.library.engine STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.engine PRIVATE
    ContextManager.cpp
    ContextManager.hpp
    ExecutionEngine.hpp
    FeatureFactory.cpp
    FeatureFactory.hpp
    FeatureInfo.cpp
    FeatureInfo.hpp
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
endif()
