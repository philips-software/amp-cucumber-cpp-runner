add_library(cucumber_cpp.library.util INTERFACE ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.util PRIVATE
    Immoveable.hpp
)

target_include_directories(cucumber_cpp.library.util INTERFACE
    ../../..
)
