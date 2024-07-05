set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber-cpp STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp PRIVATE
    Application.cpp
    Application.hpp
    BodyMacro.hpp
    Body.hpp
    Context.hpp
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
    Steps.hpp
    StepRegistry.cpp
    StepRegistry.hpp
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
    gmock
    cucumber_gherkin_lib
    cucumber-cpp.report
    cucumber-cpp.engine
    CLI11
)

target_compile_options(cucumber-cpp
    PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>

    PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

add_subdirectory(engine)
add_subdirectory(report)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
