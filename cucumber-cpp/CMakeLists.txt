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
    Hooks.cpp
    Hooks.hpp
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
    TraceTime.cpp
    TraceTime.hpp
)

target_include_directories(cucumber-cpp PUBLIC
    ./../
)

target_link_libraries(cucumber-cpp PUBLIC
    gtest
    gherkin-cpp
    nlohmann_json::nlohmann_json

    PRIVATE
    cucumber-cpp.report
)

target_compile_options(cucumber-cpp PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
)

add_subdirectory(report)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
