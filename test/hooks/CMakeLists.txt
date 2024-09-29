set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber-cpp-test.hooks OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-test.hooks PRIVATE
    Hooks.cpp
)

target_link_libraries(cucumber-cpp-test.hooks PRIVATE
    cucumber-cpp
)
