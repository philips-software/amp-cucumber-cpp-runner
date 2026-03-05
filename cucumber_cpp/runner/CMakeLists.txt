add_library(cucumber_cpp.runner STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.runner PRIVATE
    Main.cpp
)

target_link_libraries(cucumber_cpp.runner PUBLIC
    cucumber_cpp
)
