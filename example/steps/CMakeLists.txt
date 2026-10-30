set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(example.steps OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(example.steps PRIVATE
    Steps.cpp
)

target_link_libraries(example.steps PRIVATE
    cucumber_cpp.runner
    example.include
)
