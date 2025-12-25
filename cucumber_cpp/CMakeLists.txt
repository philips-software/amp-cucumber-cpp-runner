add_subdirectory(library)
add_subdirectory(runner)
add_subdirectory(example)
add_subdirectory(acceptance_test)

add_library(cucumber_cpp INTERFACE)

target_sources(cucumber_cpp INTERFACE
    CucumberCpp.hpp
)

target_include_directories(cucumber_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(cucumber_cpp INTERFACE
    cucumber_cpp.library
)
