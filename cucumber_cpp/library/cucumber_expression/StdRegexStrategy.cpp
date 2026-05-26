#include "cucumber_cpp/library/cucumber_expression/StdRegexStrategy.hpp"
#include <cstddef>
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

    std::optional<std::vector<MatchGroup>> StdRegexStrategy::Match(std::string_view text) const
    {
        std::smatch match;
        const std::string textStr(text);
        if (!std::regex_search(textStr, match, regex))
            return std::nullopt;

        std::vector<MatchGroup> result;
        result.reserve(match.size());
        for (std::size_t i = 0; i < match.size(); ++i)
        {
            const auto& m = match[i];
            const auto start = m.matched ? static_cast<std::size_t>(match.position(static_cast<std::ptrdiff_t>(i))) : std::size_t{ 0 };
            result.push_back({
                .matched = m.matched,
                .value = m.matched ? m.str() : std::string{},
                .start = start,
                .end = start + static_cast<std::size_t>(m.length()),
            });
        }
        return result;
    }
}
