set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber_cpp.integration_test.hooks OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.integration_test.hooks PRIVATE
    Hooks.cpp
)

target_link_libraries(cucumber_cpp.integration_test.hooks PRIVATE
    cucumber_cpp.library
)
