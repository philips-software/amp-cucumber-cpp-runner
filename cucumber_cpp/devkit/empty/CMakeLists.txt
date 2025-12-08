add_executable(cucumber_cpp.devkit.empty ${CCR_EXCLUDE_FROM_ALL})

target_link_libraries(cucumber_cpp.devkit.empty PRIVATE
    cucumber_cpp
    cucumber_cpp.runner
)
