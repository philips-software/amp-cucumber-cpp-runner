#ifndef CUCUMBER_EXPRESSION_RE2REGEXSTRATEGY_HPP
#define CUCUMBER_EXPRESSION_RE2REGEXSTRATEGY_HPP

#include "cucumber_cpp/library/cucumber_expression/RegexStrategy.hpp"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace re2
{
    class RE2;
}

namespace cucumber_cpp::library::cucumber_expression
{
    struct Re2RegexStrategy : RegexStrategy
    {
        explicit Re2RegexStrategy(std::string_view pattern);
        ~Re2RegexStrategy() override;

        [[nodiscard]] std::optional<std::vector<MatchGroup>> Match(std::string_view text) const override;

    private:
        std::unique_ptr<re2::RE2> re2;
    };
}

#endif
