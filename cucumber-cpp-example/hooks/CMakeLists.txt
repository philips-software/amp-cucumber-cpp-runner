set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber-cpp-example.hooks OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-example.hooks PRIVATE
    hooks.cpp
    TestHooks.cpp
)

target_link_libraries(cucumber-cpp-example.hooks PRIVATE
    cucumber-cpp-runner
)
