#ifndef CUCUMBER_EXPRESSION_REGEXSTRATEGY_HPP
#define CUCUMBER_EXPRESSION_REGEXSTRATEGY_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct RegexStrategy
    {
        RegexStrategy() = default;
        RegexStrategy(const RegexStrategy&) = delete;
        RegexStrategy& operator=(const RegexStrategy&) = delete;
        RegexStrategy(RegexStrategy&&) = delete;
        RegexStrategy& operator=(RegexStrategy&&) = delete;
        virtual ~RegexStrategy() = default;

        [[nodiscard]] virtual std::optional<std::vector<std::string>> Match(std::string_view text) const = 0;
    };
}

#endif
