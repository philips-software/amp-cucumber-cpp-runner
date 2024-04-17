set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber-cpp STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp PRIVATE
    Application.cpp
    Application.hpp
    BodyMacro.hpp
    Body.hpp
    Context.hpp
    CucumberRunner.cpp
    CucumberRunner.hpp
    FeatureRunner.cpp
    FeatureRunner.hpp
    HookRegistry.cpp
    HookRegistry.hpp
    Hooks.hpp
    HookScopes.cpp
    HookScopes.hpp
    InternalError.hpp
    OnTestPartResultEventListener.cpp
    OnTestPartResultEventListener.hpp
    Rtrim.cpp
    Rtrim.hpp
    ScenarioRunner.cpp
    ScenarioRunner.hpp
    Steps.hpp
    StepRegistry.cpp
    StepRegistry.hpp
    StepRunner.cpp
    StepRunner.hpp
    TagExpression.cpp
    TagExpression.hpp
    TagsToSet.hpp
    TraceTime.cpp
    TraceTime.hpp
)

target_include_directories(cucumber-cpp PUBLIC
    ./../
)

target_link_libraries(cucumber-cpp PUBLIC
    gtest
    cucumber_gherkin_lib

    PRIVATE
    cucumber-cpp.report
    args
)

target_compile_options(cucumber-cpp PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
)

target_compile_options(cucumber-cpp PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

add_subdirectory(report)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
