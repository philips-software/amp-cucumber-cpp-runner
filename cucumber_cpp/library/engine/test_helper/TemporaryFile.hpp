#ifndef TEST_HELPER_TEMPORARYFILE_HPP
#define TEST_HELPER_TEMPORARYFILE_HPP

#include <filesystem>
#include <fstream>
#include <string_view>

namespace cucumber_cpp::library::engine::test_helper
{
    struct TemporaryFile
    {
        TemporaryFile(std::string_view name);
        ~TemporaryFile();

        TemporaryFile& operator<<(std::string_view data);

        [[nodiscard]] std::filesystem::path Path() const;

    private:
        std::ofstream CreateOfstream();

        std::filesystem::path path;
        std::ofstream ofs{ CreateOfstream() };
    };
}

#endif
