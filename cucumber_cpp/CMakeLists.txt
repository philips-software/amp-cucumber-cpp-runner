

add_subdirectory(library)
add_subdirectory(runner)
add_subdirectory(example)
add_subdirectory(acceptance_test)

add_library(cucumber_cpp INTERFACE)

target_sources(cucumber_cpp INTERFACE
    cucumber_cpp.hpp
)

target_include_directories(cucumber_cpp INTERFACE
    ./
)

target_link_libraries(cucumber_cpp INTERFACE
    cucumber_cpp.library
)
