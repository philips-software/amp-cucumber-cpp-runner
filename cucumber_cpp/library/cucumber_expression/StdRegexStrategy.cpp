#include "cucumber_cpp/library/cucumber_expression/StdRegexStrategy.hpp"
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    StdRegexStrategy::StdRegexStrategy(std::string_view pattern)
        : regex{ std::string(pattern) }
    {
    }

    std::optional<std::vector<std::string>> StdRegexStrategy::Match(std::string_view text) const
    {
        std::smatch match;
        const std::string textStr(text);
        if (!std::regex_search(textStr, match, regex))
            return std::nullopt;

        std::vector<std::string> result;
        result.reserve(match.size());
        for (const auto& m : match)
            result.emplace_back(m.str());

        return result;
    }
}
