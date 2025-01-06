#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include <filesystem>
#include <fstream>
#include <string_view>

namespace cucumber_cpp::library::engine::test_helper
{
    TemporaryFile::TemporaryFile(std::string_view name)
        : path{ std::filesystem::temp_directory_path() / name }
    {
    }

    TemporaryFile::~TemporaryFile()
    {
        ofs.close();
        std::filesystem::remove(path);
    }

    TemporaryFile& TemporaryFile::operator<<(std::string_view data)
    {
        ofs << data; // NOLINT
        ofs.flush();
        return *this;
    }

    std::filesystem::path TemporaryFile::Path() const
    {
        return path;
    }

    std::ofstream TemporaryFile::CreateOfstream()
    {
        std::filesystem::create_directories(path.parent_path());
        return { path };
    }
}
