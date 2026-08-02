#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    const std::string featuresDir{ PLUGIN_TEST_FEATURES_DIR };
    const std::string pluginDir{ PLUGIN_TEST_PLUGIN_DIR };

    std::string PluginPath(const std::string& name)
    {
        return pluginDir + "/" + name + std::string{ cucumber_cpp::library::plugin::DynamicLibrary::PlatformExtension() };
    }

    int RunApplication(const std::vector<std::string>& extraArgs)
    {
        std::vector<std::string> argStrings;
        argStrings.emplace_back("plugin_test");
        argStrings.emplace_back("--format");
        argStrings.emplace_back("summary");

        for (const auto& arg : extraArgs)
            argStrings.emplace_back(arg);

        argStrings.emplace_back("--no-recursive");
        argStrings.emplace_back(featuresDir);

        std::vector<const char*> argv;
        argv.reserve(argStrings.size());
        for (const auto& s : argStrings)
            argv.push_back(s.c_str());

        cucumber_cpp::Application app{};
        return app.Run(static_cast<int>(argv.size()), argv.data());
    }
}

int main()
{
    const auto pluginA = PluginPath("cucumber_cpp.acceptance_test.plugin_a");
    const auto pluginB = PluginPath("cucumber_cpp.acceptance_test.plugin_b");

    // First run: plugin A (steps + custom parameter) with statically linked step
    auto result = RunApplication({ "--load", pluginA, "--tags", "@plugin_a and not @plugin_b_hook" });
    if (result != 0)
    {
        std::cerr << "FAILED: First run with plugin A\n";
        return EXIT_FAILURE;
    }

    // Second run: plugin B (hooks + steps) with statically linked step
    result = RunApplication({ "--load", pluginB, "--tags", "@plugin_b_hook and not @plugin_a" });
    if (result != 0)
    {
        std::cerr << "FAILED: Second run with plugin B\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
