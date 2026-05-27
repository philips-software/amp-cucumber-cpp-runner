#include "cucumber_cpp/library/cucumber_expression/Re2RegexStrategy.hpp"
#include "absl/strings/string_view.h"
#include "cucumber_cpp/library/cucumber_expression/RegexStrategy.hpp"
#include <cstddef>
#include <optional>
#include <re2/re2.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    Re2RegexStrategy::Re2RegexStrategy(std::string_view pattern)
        : re2{ pattern }
    {
        if (!re2.ok())
            throw std::invalid_argument(re2.error());
    }

    std::optional<Matches> Re2RegexStrategy::Match(std::string_view text) const
    {
        const int nCaptures = re2.NumberOfCapturingGroups();
        const int nSubmatch = nCaptures + 1;
        std::vector<absl::string_view> submatch(static_cast<std::size_t>(nSubmatch));

        if (!re2.Match(text, 0, static_cast<int>(text.size()), RE2::UNANCHORED, submatch.data(), nSubmatch))
            return std::nullopt;

        Matches result;
        result.reserve(static_cast<std::size_t>(nSubmatch));
        for (const auto& piece : submatch)
        {
            if (piece.data() == nullptr)
                result.emplace_back(std::nullopt);
            else
            {
                const auto start = static_cast<std::size_t>(piece.data() - text.data());
                result.emplace_back(MatchGroup{
                    .value = std::string(piece),
                    .start = start,
                    .end = start + piece.size(),
                });
            }
        }
        return result;
    }
}
