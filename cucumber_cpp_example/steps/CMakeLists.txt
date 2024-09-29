set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber-cpp-example.steps OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-example.steps PRIVATE
    Steps.cpp
)

target_link_libraries(cucumber-cpp-example.steps PRIVATE
    cucumber-cpp-runner
)
