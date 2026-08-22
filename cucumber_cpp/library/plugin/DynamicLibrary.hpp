#ifndef PLUGIN_DYNAMIC_LIBRARY_HPP
#define PLUGIN_DYNAMIC_LIBRARY_HPP

#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::plugin
{
    struct PluginError : std::runtime_error
    {
        using runtime_error::runtime_error;
    };

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
        [[nodiscard]] void* GetSymbolAddress(std::string_view name) const;

        std::filesystem::path libraryPath;
        void* handle{ nullptr }; // NOSONAR: opaque OS module handle from dlopen/LoadLibrary
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class FnPtr>
    FnPtr DynamicLibrary::GetSymbol(std::string_view name) const
    {
        static_assert(sizeof(FnPtr) == sizeof(void*), "GetSymbol requires function pointers to be the same size as void*");

        // Object-pointer to function-pointer conversion is not portable via a cast;
        // copying the bits is the POSIX-recommended, well-defined idiom for dlsym.
        void* address = GetSymbolAddress(name);
        FnPtr symbol{};
        std::memcpy(&symbol, &address, sizeof(symbol));
        return symbol;
    }
}

#endif
