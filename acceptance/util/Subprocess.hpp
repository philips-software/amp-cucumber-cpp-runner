#ifndef ACCEPTANCE_UTIL_SUBPROCESS_HPP
#define ACCEPTANCE_UTIL_SUBPROCESS_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace acceptance::util
{
    struct ProcessResult
    {
        int exitCode{};
        std::string standardOutput;
        std::string standardError;
    };

    [[nodiscard]] ProcessResult RunProcess(
        const std::filesystem::path& executable,
        const std::vector<std::string>& arguments,
        const std::filesystem::path& workingDirectory = {});
}

#endif
