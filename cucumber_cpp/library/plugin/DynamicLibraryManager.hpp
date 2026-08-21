#ifndef PLUGIN_DYNAMIC_LIBRARY_MANAGER_HPP
#define PLUGIN_DYNAMIC_LIBRARY_MANAGER_HPP

#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace cucumber_cpp::library::plugin
{
    struct DynamicLibraryManager
    {
        void Load(const std::vector<std::string>& paths);
        void UnloadAll();

    private:
        void LoadFile(const std::filesystem::path& path);
        void LoadDirectory(const std::filesystem::path& directory);

        std::vector<DynamicLibrary> libraries;
    };
}

#endif
