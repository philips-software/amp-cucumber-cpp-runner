#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibraryManager.hpp"
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cucumber_cpp::library::plugin
{
    namespace
    {
        const std::filesystem::path fixturePath{ PLUGIN_TEST_FIXTURE_PATH };
    }

    TEST(DynamicLibraryManager, throws_when_path_does_not_exist)
    {
        DynamicLibraryManager manager;

        EXPECT_THROW(manager.Load({ "/no/such/plugin.so" }), PluginError);
    }

    TEST(DynamicLibraryManager, throws_when_path_is_neither_file_nor_directory)
    {
        DynamicLibraryManager manager;

        EXPECT_THROW(manager.Load({ "/dev/null" }), PluginError);
    }

    TEST(DynamicLibraryManager, throws_when_file_lacks_plugin_extension)
    {
        const auto textFile = std::filesystem::temp_directory_path() / "ccr_not_a_plugin.txt";
        std::ofstream{ textFile } << "not a plugin";
        DynamicLibraryManager manager;

        EXPECT_THROW(manager.Load({ textFile.string() }), PluginError);

        std::filesystem::remove(textFile);
    }

    TEST(DynamicLibraryManager, throws_when_plugin_lacks_register_symbol)
    {
        DynamicLibraryManager manager;

        EXPECT_THROW(manager.Load({ fixturePath.string() }), PluginError);
    }

    TEST(DynamicLibraryManager, scans_directory_and_reports_plugin_error)
    {
        DynamicLibraryManager manager;

        EXPECT_THROW(manager.Load({ fixturePath.parent_path().string() }), PluginError);
    }

    TEST(DynamicLibraryManager, ignores_directory_without_plugins)
    {
        const auto emptyDir = std::filesystem::temp_directory_path() / "ccr_empty_plugin_dir";
        std::filesystem::create_directories(emptyDir);
        DynamicLibraryManager manager;

        EXPECT_NO_THROW(manager.Load({ emptyDir.string() }));

        std::filesystem::remove(emptyDir);
    }
}
