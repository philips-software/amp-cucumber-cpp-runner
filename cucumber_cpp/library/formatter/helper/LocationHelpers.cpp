#include "cucumber_cpp/library/formatter/helper/LocationHelpers.hpp"
#include "cucumber_cpp/library/formatter/helper/TestCaseAttemptParser.hpp"
#include <filesystem>
#include <format>
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::string FormatLocation(LineAndUri obj, std::optional<std::filesystem::path> cwd)
    {
        std::string uri = obj.uri;
        if (cwd)
            uri = std::filesystem::relative(obj.uri, *cwd).string();
        return std::format("{}:{}", uri, obj.line);
    }
}
