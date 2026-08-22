add_executable(cucumber_cpp.library.plugin.test)
add_test(NAME cucumber_cpp.library.plugin.test COMMAND cucumber_cpp.library.plugin.test)

target_link_libraries(cucumber_cpp.library.plugin.test PUBLIC
    cucumber_cpp.library
    cucumber_cpp.library.plugin
    GTest::gmock_main
)

target_sources(cucumber_cpp.library.plugin.test PRIVATE
    TestDynamicLibrary.cpp
    TestDynamicLibraryManager.cpp
    TestPluginRegister.cpp
    ${PROJECT_SOURCE_DIR}/cucumber_cpp/library/plugin/PluginRegister.cpp
)

# Standalone fixture library the tests load at runtime. Built as a plain MODULE
# (no ccr_register) so it resolves on its own and drives the loader error paths.
add_library(cucumber_cpp.library.plugin.test.fixture MODULE FixturePlugin.cpp)

target_include_directories(cucumber_cpp.library.plugin.test.fixture PRIVATE ${PROJECT_SOURCE_DIR})

set_target_properties(cucumber_cpp.library.plugin.test.fixture PROPERTIES
    CXX_VISIBILITY_PRESET default
)

if (APPLE)
    set_target_properties(cucumber_cpp.library.plugin.test.fixture PROPERTIES SUFFIX ".dylib")
endif()

add_dependencies(cucumber_cpp.library.plugin.test cucumber_cpp.library.plugin.test.fixture)

target_compile_definitions(cucumber_cpp.library.plugin.test PRIVATE
    PLUGIN_TEST_FIXTURE_PATH="$<TARGET_FILE:cucumber_cpp.library.plugin.test.fixture>"
)
