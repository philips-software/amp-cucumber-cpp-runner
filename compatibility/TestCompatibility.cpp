#include "BaseCompatibility.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace compatibility
{
    namespace
    {
        std::vector<KitInfo> DiscoverKits()
        {
            std::vector<KitInfo> kits;

            const std::filesystem::path sourceDir{ COMPAT_SOURCE_DIR };
            const std::filesystem::path pluginDir{ COMPAT_PLUGIN_DIR };

            for (const auto& entry : std::filesystem::directory_iterator{ sourceDir })
            {
                if (!entry.is_directory())
                    continue;

                const auto name = entry.path().filename().string();
                const auto ndjsonFile = entry.path() / (name + ".ndjson");

                if (!std::filesystem::exists(ndjsonFile))
                    continue;

                const auto pluginName = "compatibility." + name + ".plugin" + std::string{ cucumber_cpp::library::plugin::DynamicLibrary::PlatformExtension() };
                const auto pluginPath = pluginDir / pluginName;

                std::vector<std::string> extraArgs;
                const auto argsFile = entry.path() / (name + ".arguments.txt");
                if (std::filesystem::exists(argsFile))
                {
                    std::ifstream ifs{ argsFile };
                    std::string arg;
                    while (ifs >> arg)
                        extraArgs.push_back(arg);
                }

                kits.push_back(KitInfo{
                    .name = name,
                    .sourceDir = entry.path(),
                    .ndjsonFile = ndjsonFile,
                    .pluginPath = pluginPath,
                    .extraArgs = std::move(extraArgs),
                    .hasPlugin = std::filesystem::exists(entry.path() / (name + ".cpp")),
                });
            }

            std::ranges::sort(kits, {}, &KitInfo::name);
            return kits;
        }

        std::string KitInfoToName(const testing::TestParamInfo<KitInfo>& info)
        {
            auto name = info.param.name;
            std::ranges::replace(name, '-', '_');
            return name;
        }

        class CompatibilityTest : public testing::TestWithParam<KitInfo>
        {};
    }

    TEST_P(CompatibilityTest, Kit)
    {
        const auto& kit = GetParam();

        if (!kit.hasPlugin)
            GTEST_SKIP() << "No plugin for kit: " << kit.name;

        RunDevkit(kit);
    }

    INSTANTIATE_TEST_SUITE_P(
        Compatibility,
        CompatibilityTest,
        testing::ValuesIn(DiscoverKits()),
        KitInfoToName);
}
