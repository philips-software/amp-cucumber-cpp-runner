set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber-cpp-test.steps OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-test.steps PRIVATE
    steps.cpp
)

target_link_libraries(cucumber-cpp-test.steps PRIVATE
    cucumber-cpp
)
