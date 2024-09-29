set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_executable(cucumber-cpp-example ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-example PRIVATE
    fixtures/Fixture.hpp
    Main.cpp
)

target_link_libraries(cucumber-cpp-example PRIVATE
    cucumber-cpp
    cucumber-cpp-example.hooks
    cucumber-cpp-example.steps
)

add_subdirectory(hooks)
add_subdirectory(steps)
