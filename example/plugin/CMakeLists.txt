include(${PROJECT_SOURCE_DIR}/cmake/ccr_add_plugin.cmake)

ccr_add_plugin(example.plugin)

target_sources(example.plugin PRIVATE
    PluginHooks.cpp
    PluginSteps.cpp
)

target_link_libraries(example.plugin PRIVATE
    example.include
)
