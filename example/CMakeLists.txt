set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(example.include INTERFACE)
target_include_directories(example.include INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(example ${CCR_EXCLUDE_FROM_ALL})

target_sources(example PRIVATE
    fixtures/Fixture.hpp
    Main.cpp
)

target_link_libraries(example PRIVATE
    cucumber_cpp
    example.hooks
    example.steps
    example.include
)

set_target_properties(example PROPERTIES
    ENABLE_EXPORTS ON
)

add_subdirectory(hooks)
add_subdirectory(plugin)
add_subdirectory(steps)
