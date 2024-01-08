set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_executable(cucumber-cpp-example ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-example PRIVATE
    fixtures/Fixture.hpp
    hooks/hooks.cpp
)

target_link_libraries(cucumber-cpp-example PRIVATE
    cucumber-cpp-runner
    cucumber-cpp-example.steps
)

add_subdirectory(steps)
