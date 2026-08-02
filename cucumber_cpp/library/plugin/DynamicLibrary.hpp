#ifndef PLUGIN_DYNAMIC_LIBRARY_HPP
#define PLUGIN_DYNAMIC_LIBRARY_HPP

#include <filesystem>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::plugin
{
    struct DynamicLibrary
    {
        explicit DynamicLibrary(const std::filesystem::path& path);

        ~DynamicLibrary();

        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        DynamicLibrary(DynamicLibrary&& other) noexcept;
        DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

        template<class FnPtr>
        FnPtr GetSymbol(std::string_view name) const;

        [[nodiscard]] const std::filesystem::path& Path() const;

        static std::string_view PlatformExtension();

    private:
        void* GetSymbolAddress(std::string_view name) const;

        std::filesystem::path libraryPath;
        void* handle{ nullptr };
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class FnPtr>
    FnPtr DynamicLibrary::GetSymbol(std::string_view name) const
    {
        return reinterpret_cast<FnPtr>(GetSymbolAddress(name)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    }
}

#endif
