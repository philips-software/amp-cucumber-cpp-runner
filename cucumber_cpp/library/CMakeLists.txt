add_library(cucumber_cpp.library ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library PRIVATE
    Application.cpp
    Application.hpp
    BodyMacro.hpp
    Context.hpp
    Errors.hpp
    Hooks.hpp
    Parameter.hpp
    Steps.hpp
)

target_include_directories(cucumber_cpp.library PUBLIC
    ../..
)

target_link_libraries(cucumber_cpp.library PUBLIC
    CLI11::CLI11
    cucumber_cpp.library.api
    cucumber_cpp.library.engine
    cucumber_cpp.library.formatter
    cucumber_cpp.library.plugin
    cucumber_cpp.library.support
    cucumber_cpp.library.util
    cucumber_gherkin_lib
    cucumber::tag_expressions
    GTest::gmock
    GTest::gtest
)

target_compile_options(cucumber_cpp.library
    PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>

    PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

add_subdirectory(api)
add_subdirectory(assemble)
add_subdirectory(cucumber_expression)
add_subdirectory(engine)
add_subdirectory(formatter)
add_subdirectory(plugin)
add_subdirectory(runtime)
add_subdirectory(support)
add_subdirectory(util)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
