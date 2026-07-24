include(${PROJECT_SOURCE_DIR}/cmake/ccr_add_plugin.cmake)

ccr_add_plugin(cucumber_cpp.example.plugin)

target_sources(cucumber_cpp.example.plugin PRIVATE
    PluginSteps.cpp
    PluginHooks.cpp
)
