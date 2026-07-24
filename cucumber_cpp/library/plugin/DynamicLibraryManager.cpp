#include "cucumber_cpp/library/plugin/DynamicLibraryManager.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace cucumber_cpp::library::plugin
{
    void DynamicLibraryManager::Load(const std::vector<std::string>& paths)
    {
        for (const auto& path : paths)
        {
            const std::filesystem::path fsPath{ path };

            if (std::filesystem::is_directory(fsPath))
                LoadDirectory(fsPath);
            else if (std::filesystem::is_regular_file(fsPath))
                LoadFile(fsPath);
            else
                throw std::runtime_error("Path '" + path + "' is not a file or directory");
        }
    }

    void DynamicLibraryManager::UnloadAll()
    {
        libraries.clear();
    }

    std::vector<std::filesystem::path> DynamicLibraryManager::GetLoadedLibraries() const
    {
        std::vector<std::filesystem::path> paths;
        paths.reserve(libraries.size());

        for (const auto& lib : libraries)
            paths.push_back(lib.Path());

        return paths;
    }

    void DynamicLibraryManager::LoadFile(const std::filesystem::path& path)
    {
        const auto& lib = libraries.emplace_back(path);

        auto registerFn = lib.GetSymbol<CcrRegisterFn>("ccr_register");

        PluginHostContext context{
            .registration = &support::DefinitionRegistration::Instance(),
            .stopwatch = &util::Stopwatch::Instance(),
            .timestampGenerator = &util::TimestampGenerator::Instance(),
            .converterMap = &cucumber_expression::ConverterRegistry::Instance(),
        };

        registerFn(&context);
    }

    void DynamicLibraryManager::LoadDirectory(const std::filesystem::path& directory)
    {
        const auto extension = DynamicLibrary::PlatformExtension();

        std::vector<std::filesystem::path> sortedPaths;

        for (const auto& entry : std::filesystem::directory_iterator{ directory })
            if (std::filesystem::is_regular_file(entry) && entry.path().extension() == extension)
                sortedPaths.push_back(entry.path());

        std::ranges::sort(sortedPaths);

        for (const auto& path : sortedPaths)
            LoadFile(path);
    }
}
