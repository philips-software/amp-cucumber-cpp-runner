#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace cucumber_cpp::library::plugin
{
    namespace
    {
#if defined(_WIN32)
        std::string GetLastErrorMessage()
        {
            const DWORD error = GetLastError();

            if (error == 0)
                return "Unknown error";

            LPSTR buffer = nullptr;
            const auto size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&buffer), 0, nullptr); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

            if (size == 0 || buffer == nullptr)
                return "Unknown error (code " + std::to_string(error) + ")";

            std::string message(buffer, size);
            LocalFree(buffer);
            return message;
        }
#endif
    }

    DynamicLibrary::DynamicLibrary(const std::filesystem::path& path)
        : libraryPath{ path }
    {
#if defined(_WIN32)
        handle = LoadLibraryW(path.c_str());

        if (handle == nullptr)
            throw std::runtime_error("Failed to load library '" + path.string() + "': " + GetLastErrorMessage());
#else
        handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL); // NOLINT(hicpp-signed-bitwise)

        if (handle == nullptr)
            throw std::runtime_error("Failed to load library '" + path.string() + "': " + dlerror());
#endif
    }

    DynamicLibrary::~DynamicLibrary()
    {
        if (handle != nullptr)
        {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
        }
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
        : libraryPath{ std::move(other.libraryPath) }
        , handle{ std::exchange(other.handle, nullptr) }
    {}

    DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept
    {
        if (this != &other)
        {
            if (handle != nullptr)
            {
#if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(handle));
#else
                dlclose(handle);
#endif
            }

            libraryPath = std::move(other.libraryPath);
            handle = std::exchange(other.handle, nullptr);
        }

        return *this;
    }

    const std::filesystem::path& DynamicLibrary::Path() const
    {
        return libraryPath;
    }

    std::string_view DynamicLibrary::PlatformExtension()
    {
#if defined(_WIN32)
        return ".dll";
#elif defined(__APPLE__)
        return ".dylib";
#else
        return ".so";
#endif
    }

    void* DynamicLibrary::GetSymbolAddress(std::string_view name) const
    {
        const std::string symbolName{ name };

#if defined(_WIN32)
        void* symbol = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), symbolName.c_str())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

        if (symbol == nullptr)
            throw std::runtime_error("Symbol '" + symbolName + "' not found in library '" + libraryPath.string() + "': " + GetLastErrorMessage());
#else
        dlerror(); // clear previous error
        void* symbol = dlsym(handle, symbolName.c_str());
        const char* error = dlerror();

        if (error != nullptr)
            throw std::runtime_error("Symbol '" + symbolName + "' not found in library '" + libraryPath.string() + "': " + error);
#endif

        return symbol;
    }
}
