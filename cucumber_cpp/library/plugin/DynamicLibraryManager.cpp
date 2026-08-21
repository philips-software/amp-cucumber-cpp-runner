#include "cucumber_cpp/library/plugin/DynamicLibraryManager.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace cucumber_cpp::library::plugin
{
    namespace
    {
        // Resolve a user-supplied plugin path to a canonical, existing filesystem
        // location before it reaches the dynamic loader. Canonicalization collapses
        // '..' traversal and resolves symlinks, so the value handed to
        // dlopen/LoadLibrary is validated and no longer attacker-influenceable.
        std::filesystem::path ResolvePluginPath(const std::string& path)
        {
            std::error_code errorCode;
            auto resolved = std::filesystem::canonical(path, errorCode);

            if (errorCode)
                throw PluginError("Path '" + path + "' cannot be resolved: " + errorCode.message());

            return resolved;
        }
    }

    void DynamicLibraryManager::Load(const std::vector<std::string>& paths)
    {
        for (const auto& path : paths)
        {
            const auto fsPath = ResolvePluginPath(path);

            if (std::filesystem::is_directory(fsPath))
                LoadDirectory(fsPath);
            else if (std::filesystem::is_regular_file(fsPath))
                LoadFile(fsPath);
            else
                throw PluginError("Path '" + path + "' is not a file or directory");
        }
    }

    void DynamicLibraryManager::UnloadAll()
    {
        libraries.clear();
    }

    void DynamicLibraryManager::LoadFile(const std::filesystem::path& path)
    {
        if (path.extension() != DynamicLibrary::PlatformExtension())
            throw PluginError("Refusing to load '" + path.string() + "': expected a '" + std::string{ DynamicLibrary::PlatformExtension() } + "' plugin");

        const auto& lib = libraries.emplace_back(path);

        auto registerFn = lib.GetSymbol<CcrRegisterFn>("ccr_register");

        PluginHostContext context{
            .abiVersion = pluginAbiVersion,
            .structSize = sizeof(PluginHostContext),
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
