#ifndef CUCUMBER_EXPRESSION_STDREGEXSTRATEGY_HPP
#define CUCUMBER_EXPRESSION_STDREGEXSTRATEGY_HPP

#include "cucumber_cpp/library/cucumber_expression/RegexStrategy.hpp"
#include <optional>
#include <regex>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct StdRegexStrategy : RegexStrategy
    {
        explicit StdRegexStrategy(std::string_view pattern);

        [[nodiscard]] std::optional<std::vector<MatchGroup>> Match(std::string_view text) const override;

    private:
        std::regex regex;
    };
}

#endif
