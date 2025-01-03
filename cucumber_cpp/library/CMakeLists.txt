set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber_cpp.library STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library PRIVATE
    Application.cpp
    Application.hpp
    BodyMacro.hpp
    Body.hpp
    Context.hpp
    HookRegistry.cpp
    HookRegistry.hpp
    Hooks.hpp
    InternalError.hpp
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

target_include_directories(cucumber_cpp.library PUBLIC
    ../..
)

target_link_libraries(cucumber_cpp.library PUBLIC
    gtest
    gmock
    cucumber_gherkin_lib
    cucumber_cpp.library.report
    cucumber_cpp.library.engine
    cucumber_cpp.library.util
    CLI11
)

target_compile_options(cucumber_cpp.library
    PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>

    PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

add_subdirectory(engine)
add_subdirectory(report)
add_subdirectory(util)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
