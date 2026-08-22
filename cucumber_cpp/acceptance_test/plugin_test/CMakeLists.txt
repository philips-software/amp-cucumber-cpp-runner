include(${PROJECT_SOURCE_DIR}/cmake/ccr_add_plugin.cmake)

# Test executable with statically linked step that loads plugins
add_executable(cucumber_cpp.acceptance_test.plugin ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.acceptance_test.plugin PRIVATE
    StaticSteps.cpp
    MainPlugin.cpp
)

target_link_libraries(cucumber_cpp.acceptance_test.plugin PRIVATE
    cucumber_cpp
)

set_target_properties(cucumber_cpp.acceptance_test.plugin PROPERTIES
    ENABLE_EXPORTS ON
)

file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/features" PLUGIN_TEST_FEATURES_DIR_CMAKE)

target_compile_definitions(cucumber_cpp.acceptance_test.plugin PRIVATE
    PLUGIN_TEST_FEATURES_DIR="${PLUGIN_TEST_FEATURES_DIR_CMAKE}"
    PLUGIN_TEST_PLUGIN_DIR="$<TARGET_FILE_DIR:cucumber_cpp.acceptance_test.plugin>"
)

if (NOT WIN32)
    target_link_libraries(cucumber_cpp.acceptance_test.plugin PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,cucumber_cpp.library,cucumber_cpp.library.api,cucumber_cpp.library.engine,cucumber_cpp.library.support,cucumber_cpp.library.util,GTest::gtest,GTest::gmock>
    )
endif()

# Plugin A: steps + custom parameter type
ccr_add_plugin(cucumber_cpp.acceptance_test.plugin_a PluginA.cpp)

# Plugin B: hooks + steps
ccr_add_plugin(cucumber_cpp.acceptance_test.plugin_b PluginB.cpp)
