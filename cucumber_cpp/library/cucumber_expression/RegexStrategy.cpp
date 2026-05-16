#include "cucumber_cpp/library/cucumber_expression/RegexStrategy.hpp"
#include <re2/re2.h>
#include <optional>
#include <regex>
#include <stdexcept>
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

    Re2RegexStrategy::Re2RegexStrategy(std::string_view pattern)
        : re2{ std::make_unique<re2::RE2>(pattern) }
    {
        if (!re2->ok())
            throw std::invalid_argument(re2->error());
    }

    Re2RegexStrategy::~Re2RegexStrategy() = default;

    std::optional<std::vector<std::string>> Re2RegexStrategy::Match(std::string_view text) const
    {
        const int nCaptures = re2->NumberOfCapturingGroups();
        const int nSubmatch = nCaptures + 1;
        std::vector<absl::string_view> submatch(static_cast<std::size_t>(nSubmatch));

        if (!re2->Match(text, 0, static_cast<int>(text.size()), RE2::UNANCHORED, submatch.data(), nSubmatch))
            return std::nullopt;

        std::vector<std::string> result;
        result.reserve(static_cast<std::size_t>(nSubmatch));
        for (const auto& piece : submatch)
            result.emplace_back(piece.data(), piece.size());

        return result;
    }
}
