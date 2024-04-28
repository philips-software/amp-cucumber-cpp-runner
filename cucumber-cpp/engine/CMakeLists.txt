add_library(cucumber-cpp.engine STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp.engine PRIVATE
    AmbiguousStepInfo.cpp
    AmbiguousStepInfo.hpp
    ExecutionEngine.hpp
    FeatureFactory.cpp
    FeatureFactory.hpp
    FeatureInfo.cpp
    FeatureInfo.hpp
    MissingStepInfo.hpp
    ScenarioInfo.cpp
    ScenarioInfo.hpp
    StepInfo.cpp
    StepInfo.hpp
    StepInfoBase.cpp
    StepInfoBase.hpp
    TestRunner.cpp
    TestRunner.hpp
)

target_include_directories(cucumber-cpp.engine PUBLIC
    ./../../
)

target_link_libraries(cucumber-cpp.engine PUBLIC
    cucumber-cpp
)
