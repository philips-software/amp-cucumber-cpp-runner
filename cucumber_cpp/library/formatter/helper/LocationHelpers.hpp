#ifndef HELPER_FORMAT_LOCATION_HPP
#define HELPER_FORMAT_LOCATION_HPP

#include "cucumber_cpp/library/formatter/helper/TestCaseAttemptParser.hpp"
#include <filesystem>
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{

    std::string FormatLocation(LineAndUri obj, std::optional<std::filesystem::path> cwd = std::nullopt);
}

#endif
