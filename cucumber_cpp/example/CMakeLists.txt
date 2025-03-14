set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_executable(cucumber_cpp.example ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.example PRIVATE
    fixtures/Fixture.hpp
    Main.cpp
)

target_link_libraries(cucumber_cpp.example PRIVATE
    cucumber_cpp
    cucumber_cpp.example.hooks
    cucumber_cpp.example.steps
)

add_subdirectory(hooks)
add_subdirectory(steps)
