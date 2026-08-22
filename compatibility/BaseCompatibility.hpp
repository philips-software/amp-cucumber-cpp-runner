#ifndef COMPATIBILITY_BASE_COMPATIBILITY_HPP
#define COMPATIBILITY_BASE_COMPATIBILITY_HPP

#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace compatibility
{
    struct KitInfo
    {
        std::string name;
        std::filesystem::path sourceDir;
        std::filesystem::path buildDir;
        std::filesystem::path ndjsonFile;
        std::filesystem::path pluginPath;
        std::vector<std::string> extraArgs;
        bool hasPlugin;
    };

    inline void PrintTo(const KitInfo& kit, std::ostream* os)
    {
        *os << kit.name;
    }

    void RunDevkit(const KitInfo& kit);
}

#endif
