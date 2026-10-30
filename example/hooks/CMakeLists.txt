set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(example.hooks OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(example.hooks PRIVATE
    Hooks.cpp
    TestHooks.cpp
)

target_link_libraries(example.hooks PRIVATE
    cucumber_cpp.runner
    example.include
)
