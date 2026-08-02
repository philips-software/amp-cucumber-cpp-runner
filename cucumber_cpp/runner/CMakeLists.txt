add_library(cucumber_cpp.runner STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.runner PRIVATE
    Main.cpp
)

target_link_libraries(cucumber_cpp.runner PUBLIC
    cucumber_cpp
)

# Note: executables that link cucumber_cpp.runner and use --load for plugins
# must set ENABLE_EXPORTS ON to export symbols for dynamic library resolution:
#   set_target_properties(my_executable PROPERTIES ENABLE_EXPORTS ON)
