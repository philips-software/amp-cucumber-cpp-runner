#include <cstddef>
#include <filesystem>
#include <functional>
#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <set>
#include <string>

namespace compatibility
{
    struct Devkit
    {
        std::set<std::filesystem::path, std::less<>> paths;
        std::string tagExpression;
        std::size_t retry;
        std::filesystem::path ndjsonFile;
        std::string folder;
        std::string kitString;
    };

    void RunDevkit(Devkit devkit);
}
